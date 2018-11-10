#include "EERTOS.h"
#include "EERTOSHAL.h"
#include "EERTOS_DEBUG.h"

//#undef DEBUG
/*
-Насколько хорошо это будет работать, если таймер установить на 50-100мкс при 16МГц?

DI HALT: 9 Апрель 2012 в 21:08
Плохо будет работать. Там довольно многое можно оптимизировать,
среднее выполнение блока таймерной службы сейчас около 600 тактов.
Ну или около того. Столько же в среднем занимает перебор очереди,
установка таймера порядка 700 тактов, установка задачи около 400 тактов.
//=150uS - время на предподготовку задачи (примерное из предыдущ. днн^)
Реально тайминг снизить до 500мкс ,но делать очень короткие задачи, иначе таймер лажать будет.
*/


//-----------------------------------------------------------------------------------
//(Для AVR - старая версия/ Частота ядра 8 или 16MHZ. Период вызова диспетчера 1мс)
//Ядро:
//KERNEL_Sort_TaskQueue();  300us //Запускается редко
//TaskManager();            150us
//SetTimerTask();           180us
//Таймерное прерывание 1мс:
//TimerService();           60us
//KERNEL_CorpseService();   180us
/*
//Кол-во задач =            10;
//Установка 1 задачи =      10 байт
//+
//Сама структура =          13 байт
//= 1_Задача в RAM =        23 байта
ATmega128 memory use summary [bytes]:
Segment   Begin    End      Code   Data   Used    Size   Use%
---------------------------------------------------------------
[.cseg] 0x000000 0x000af4   2626    178   2804  131072   2.1%
[.dseg] 0x000100 0x0005ca      0    202    202    4351   4.6%
*/
//-----------------------------------------------------------------------------------


 volatile static TASK_STRUCT  TTask[TASK_QUEUE_SIZE+1];    // Очередь таймеров

 volatile static uint8_t timers_cnt_tail = 0;
 volatile uint32_t v_u32_SYS_TICK;
 volatile uint16_t Global_delay_defth = 0;
 volatile bit TASK_RDY_TO_DISPATCH = 1;//Флаг разрешения прочёсывания очереди диспетчером, устанавливается из TimerService

	 volatile uint8_t NEED_TO_SORT = 1;//Флаг разрешения сортировки очереди задач после добавления/удаления

 #ifdef USE_TTASKS_LOGGING
	#warning - при учащённом вызове задач лог нужно скидывать тоже чаще, иначе переполнится!!!!
	 volatile uint32_t v_u32_SYS_TICK_TMP_Start = 0;
	 volatile uint32_t v_u32_SYS_TICK_TMP_End   = 0;
	 volatile uint32_t Idle_statts_cnt = 0;

	 T_PTR Shed_CurrentTask = NULL;
	 T_PTR TMP_CurrentTask = NULL;
	 uint32_t cycle_time_start = 0;
	 uint32_t cycle_time_end = 0;
	 uint32_t cycle_time_elapsed = 0;
 #endif

 #ifdef USE_CORPSE_SERVISE
	volatile bit InfiniteLoopFlag = 1; //Если задача зависнет - то в прерывании об этом узнают и прибьют по таймауту!
 #endif

//+++++++++++++PRIVATE RTOS INT VARS+++++++++++++++++++++++++++++++
extern uint32_t CPSR_;
extern bit	       global_nointerrupted_flag;
extern U_ALU_INT      global_interrupt_mask;
extern U_ALU_INT      global_interrupt_cond;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++PRIVATE RTOS SERVICES++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  inline void KERNEL_Sort_TaskQueue (void); //Сортировка задач по периоду (выполняется фоном)
  inline void KERNEL_Get_TaskStats (uint16_t index);//получение статистики задачи
  void TaskName_Assign (T_PTR task, char* name);
	
	static void TaskSetToNULL(uint16_t index); //зануление атрибутов задачи по индексу
  static void clear_duplicates (void); //not tested TODO:

#ifdef USE_TTASKS_LOGGING
  static uint32_t CalkElapsed(uint32_t start, uint32_t stop);
  inline static void HandlerProfiling_Start(void);
  inline static void HandlerProfiling_End(void);  
#endif// USE_TTASKS_LOGGING
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//===============================================================================================
 void RTOS_TIMER_ISR_HNDL(void)
  {
    v_u32_SYS_TICK++;
    TimerService();
  #ifdef USE_CORPSE_SERVISE
    KERNEL_CorpseService(); //очистка от зависших задач
  #endif
  #ifdef USE_USER_HOOKS
    USER_RtosTimerIsrHook();
  #endif
  }
//===============================================================================================

//===============================================================================================

 void Shedull (uint8_t param)
 {
	 while(1)
	 {
#ifdef USE_TTASKS_LOGGING		 
	 HandlerProfiling_Start(); //выяснить странность статистики
#endif// USE_TTASKS_LOGGING
		 
		  TaskManager();
		  ProcessTasksMessages();
		 
#ifdef USE_TTASKS_LOGGING			 
	 HandlerProfiling_End();
#endif// USE_TTASKS_LOGGING
	 if(param == 0){break;}
	}
}
//===============================================================================================

 /*
 *                                          OSIdleCtr
 *                 OSCPUUsage = 100 * (1 - ------------)     (units are in %)
 *                                         OSIdleCtrMax
 */
//===============================================================================================
 long t1, t2, t3;
 long avg_idle_time = 0;
 long avg_idle_time_tmp = 0;
 long avg_cnt = 0;
 volatile uint32_t Idle_statts_cnt_global = 0;
 long Abs_in_Idle_time = 0;

DECLARE_TASK (Idle)   //Пустая процедура - простой ядра.
{
  static volatile int In_Idle = 0;
#ifdef USE_TTASKS_LOGGING
	t1 = GET_TSC;
#endif

#ifdef DEBUG
	_LED_B_OFF;   //Для отслеживания загрузки системы
#endif
	
  #ifdef USE_SORTING_TTASK_QUEUE
	if(v_u32_SYS_TICK % QUEUE_SORTING_PERIOD == 0)//За 1 тик можно попасть сюда много раз!
        {
          if(In_Idle == 0) //Это защита от бесполезной работы
          {
            if(NEED_TO_SORT)//Только когда добавили или удалили задачу!
            {
              KERNEL_Sort_TaskQueue();//Периодическое упорядочевание задач по длительости переиода
              NEED_TO_SORT = 0; //Запрещаем сортировку задач, уже отсортированы
            }

          avg_idle_time = avg_idle_time_tmp/avg_cnt;
						Abs_in_Idle_time = (avg_idle_time * Idle_statts_cnt_global)/QUEUE_SORTING_PERIOD/2;//2 - sys clk MHz //TODO: сделать неконстантную привязку!!!
          Idle_statts_cnt_global  = 0;
          avg_idle_time_tmp = 0;
          avg_cnt = 0;
          In_Idle = 1;
          }
        }
        else {In_Idle = 0;}
    #endif //USE_SORTING_TTASK_QUEUE
#ifdef USE_USER_HOOKS
    USER_IdleHook();
#endif
#ifdef USE_TTASKS_LOGGING
	Idle_statts_cnt++; //счётчик пребывания в Idle до следующей задачи
	//if(Idle_statts_cnt > LONG_IDLE_PERIOD){SysTick_Handler(); Idle_statts_cnt = 0;/*проверить сё ли хорошо в системе, если надо - перезагрузить!*/}
 t2 = GET_TSC;
t3 = t2 - t1;
avg_idle_time_tmp += t3;
avg_cnt++;
#endif //USE_TTASKS_LOGGING
}
//===============================================================================================

 //TODO: look at http://we.easyelectronics.ru/Soft/minimalistichnaya-ochered-zadach-na-c.html
 //TODO: look at http://we.easyelectronics.ru/Soft/dispetcher-snova-dispetcher.html
 //TODO: добавить атомарности при манипуляциях с тасками!!!
 //http://habrahabr.ru/post/58366/ event-driven system!!!
 //http://www.femtoos.org/features.html!!!!
 //http://www.cs.ucr.edu/~vahid/rios/ различные варианты диспетчера
 //!!!http://habrahabr.ru/post/267573/ TNeo RTOS!!!!!!!!! SRC https://bitbucket.org/dfrank/tneokernel/src/56a3cacc9e1d?at=default
//===============================================================================================
//Диспетчер задач ОС. Выбирает из очереди задачи и отправляет на выполнение
//===============================================================================================
extern DECLARE_TASK(T_AxelNew);//for debug

static void TaskManager(void) //в диспетчере задач выполняются задачи из очереди TTask, которые уже "выщелкали"
{
U_ALU_INT	index = 0;
T_PTR 		CurrentTask;

#ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
	T_ARG       TaskArg;
	T_RET       TaskRetVal;
#endif

_disable_interrupts();
if(TASK_RDY_TO_DISPATCH)//Можно запускать диспетчер и прочёсывать очередь, задача установлена!
{
 FOREACH_TASK // Прочесываем очередь задач
 {
  if ((TTask[index].TStatus == RDY)) // Если задача отмечена к выполнению (пропускаем пустые и те, время которых еще не подошло)
	{
		 CurrentTask = TTask[index].GoToTask;  // запомним задачу т.к. во время выполнения может измениться индекс
		 TTask[index].TStatus = IN_PROC;  //Задача в процессе выполнения
		 if(TTask[index].TPeriod != 0){TTask[index].TDelay = TTask[index].TPeriod-1;} //перезапись задержки
		 TASK_RDY_TO_DISPATCH = 0;//Задача поставлена на выполнение

	#ifdef DEBUG
		 if(CurrentTask != Idle)// фиксируем загрузку процессора не Idle-задачами
		 {
			  _LED_B_ON;   //Для отслеживания загрузки системы
		 
		 #ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
			  TaskArg = (T_ARG)index; //for ret/arg tests
		 #endif
		 }
	#endif

	#ifdef USE_TTASKS_LOGGING                                    //запись св-ств задачи для лога
				TTask[index].sys_tick_time = v_u32_SYS_TICK; //время начала выполнения
				Shed_CurrentTask = TTask[index].GoToTask;  // запомним задачу для логов
				v_u32_SYS_TICK_TMP_Start = GET_TSC; 	//засекаем время выполнения задачи
	#endif
	//---------------------------Дальше идём на выполнение задачи-----------------------------------------------
	//InfiniteLoopFlag = 1; // Предположительно (Устанавливается в KERNEL_CorpseService in RTOS timer ISR)
		_enable_interrupts();							// Разрешаем прерывания
	#ifdef USE_VOID_TASK_VS_VOID_PARAMETER
			   (CurrentTask)();			       					 // ПЕРЕХОД К ЗАДАЧЕ!
	#endif
	#ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
			   TaskRetVal = (CurrentTask)(TaskArg);  			 // ПЕРЕХОД К ЗАДАЧЕ!
	#endif
		 
	#ifdef USE_CORPSE_SERVISE
	InfiniteLoopFlag = 0; //Если задача зависнет - то в прерывании об этом узнают и прибьют по таймауту!
	#endif
	//----------------------------------------------------------------------------------------------------
	#warning при возвращении после задержки если произошла перестановка задач - индекс уже не тот!!!
	//нужно снова отыскать задачу по CurrentTask
	FOREACH_TASK{ if((TTask[index].GoToTask == CurrentTask)){ break;} }

	if(TTask[index].TStatus != DEAD) //Если задача не была отмечена в прерывании как зависшая
	{
	 TTask[index].TStatus = DONE; //меняем статус - благополучно выполнилась!
	}

	#ifdef USE_TTASKS_LOGGING

	 v_u32_SYS_TICK_TMP_End = GET_TSC; 	//засекаем время выполнения задачи

	 if(v_u32_SYS_TICK_TMP_Start <= v_u32_SYS_TICK_TMP_End)
	 {
		v_u32_SYS_TICK_TMP_Start = v_u32_SYS_TICK_TMP_End - v_u32_SYS_TICK_TMP_Start;
	 }
	 else //решение вопроса при переполнении
	 {
		v_u32_SYS_TICK_TMP_Start = v_u32_SYS_TICK_TMP_Start - v_u32_SYS_TICK_TMP_End;
	 }

	  TTask[index].exec_time = v_u32_SYS_TICK_TMP_Start/72;//запишем время её выполнения - /72 in uS
	  TTask[index].run_me_cnt--;      //задача выполнилась -- раз

	   KERNEL_Get_TaskStats(index);
	   Idle_statts_cnt_global += Idle_statts_cnt;//сумма вхождений в Idle для расчёта времени
	   Idle_statts_cnt = 0; //сброс счётчика Idle-циклов
	   v_u32_SYS_TICK_TMP_Start = 0;
	   v_u32_SYS_TICK_TMP_End = 0;
	#endif
			   // Если код выхода задачи не 0 - обработать!
	 #ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
	   if(TaskRetVal){KERNEL_TaskRetHandler(CurrentTask, TaskRetVal);}
	 #endif

	   if(TTask[index].TPeriod == 0) //если период 0 - удаляем задачу из списка
	   {
		 ClearTimerTask(CurrentTask);  // задачи больше не существует
	   }
	_enable_interrupts();//на всякий
		 return;                             // выход до следующего цикла
  }//end if ((TTask[index].TStatus == RDY))
 }//end FOREACH_TASK							// Разрешаем прерывания
}//end if(TASK_RDY_TO_DISPATCH)
_enable_interrupts();

// если ни одна задача не готова к запуску - простой, выполнение фоновых служб, сон до следующего прерывания таймера...
#ifdef USE_VOID_TASK_VS_VOID_PARAMETER
	Idle();
#endif
#ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
     Idle(0); // TODO: подумать над передачей в Idle чего-то полезного
#endif
}
//===============================================================================================

//===============================================================================================
void T_Delay(uint32_t delay_ticks)
{
#warning Strongly not recomended to use!!!
uint32_t tick_cnt = v_u32_SYS_TICK+delay_ticks;
T_PTR DelayedTask = 0;
uint32_t index=0;
if(Global_delay_defth < GLOBAL_DELAY_DEFTH_LIMIT)
 {
	ATOMIC_BLOCK_FORCEON
	{
	FOREACH_TASK  // Прочесываем очередь задач
	 {
	 if (TTask[index].TStatus == IN_PROC) //Если задача сейчас выполняется
		{
		TTask[index].TStatus = DELAYED;//Помечаем как задержаную
		DelayedTask = TTask[index].GoToTask; //запомним задачу, дабы после задержки вернуть статус IN_PROC
		Global_delay_defth++;//Глубина вложенности задержек TODO: необходимо нормальное переключение контекста
		break;
		}// end if (TTask[index].TStatus == IN_PROC) 
	 }//end FOREACH_TASK
	}//end ATOMIC_BLOCK_FORCEON
//-----------------------------------------------------------------------------------------
	while (v_u32_SYS_TICK < tick_cnt) //пока не выщелкали всю задержку
	{
		Shedull(0);//TaskManager(); //выполняем следующие задачи
	}
//-----------------------------------------------------------------------------------------
//TODO: можно добавить проверку на удаление задачи или изменение статуса извне, тогда ошибка
	ATOMIC_BLOCK_FORCEON
	{
	FOREACH_TASK  // Прочесываем очередь задач
	 {
	 if (TTask[index].GoToTask == DelayedTask) //Если нашли ранее задержаную задачу
		{
		TTask[index].TStatus = IN_PROC;//Восстанавливаем статус
		Global_delay_defth--; //Глубина вложенности задержек TODO: необходимо нормальное переключение контекста
		break;
		}
	 }//end FOREACH_TASK
	}//end ATOMIC_BLOCK_FORCEON
 }
 else //если есть опасность переполнение стека вложеными задержками-диспетчерами-задачами..
 {
 	while (v_u32_SYS_TICK < tick_cnt) //пока не выщелкали всю задержку
	{ //похоже в єтом месте sys-tick останавливается!
#ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER	 
		Idle(0); //..тупим в Idle
#endif		
#ifdef USE_VOID_TASK_VS_VOID_PARAMETER	 
		Idle(); //..тупим в Idle
#endif		
 		//ErrorHndl ();
	 //Global_delay_defth--;
	 //return; //TODO: выбрать надёжный от зависания вариант
	}//end while
 }//end else
}//end T_Delay()
//===============================================================================================



//===============================================================================================
  void InitRTOS(void) // RTOS Подготовка. Очистка очередей
{
uint8_t	index;
RTOS_timer_init(); //Hardware!

    for(index=0;index!=TASK_QUEUE_SIZE+1;index++) // Обнуляем все таймеры.
    {
    	TaskSetToNULL(index);
    }
}
//===============================================================================================

//То же самое, что SetTimerTask олько без возвращаемых параметров и автоустановкой игнорирования 
//долгого выполнения (зависания) задачи
void SetTimerTaskInfin(T_PTR TS, unsigned int Prolongation, unsigned int NewPeriod) 
{
	  SetTimerTask(TS, Prolongation, NewPeriod); 
		SetTaskDeadtime(TS, 0xFF);
}
//===============================================================================================
 void SetTask(T_PTR TS)  // Поставить задачу в очередь для немедленного выполнения
{
 SetTimerTask(TS,0,0);
}
//===============================================================================================

//TODO: debug!
void TaskSuspend(T_PTR TS)
{
	uint8_t	 	index=0;
ATOMIC_BLOCK_FORCEON
 {
   FOREACH_TASK
    {
      if(TTask[index].GoToTask == TS)
      {
        TTask[index].TStatus = SYSPENDED;  // 
        NEED_TO_SORT = 1; //разрешить сортировку задач!
        goto exit;//exit of for-cycle
      }
    }//end FOREACH_TASK
exit: ;
 }//end of  ATOMIC_BLOCK_
}

//TODO: debug!
void TaskResume(T_PTR TS)
{
		uint8_t	 	index=0;
ATOMIC_BLOCK_FORCEON
 {
   FOREACH_TASK
    {
      if(TTask[index].GoToTask == TS)
      {
        TTask[index].TStatus = WAIT;  // 
        NEED_TO_SORT = 1; //разрешить сортировку задач!
        goto exit;//exit of for-cycle
      }
    }//end FOREACH_TASK
exit: ;
 }//end of  ATOMIC_BLOCK_
}

//Функция установки задачи по таймеру. 
//Передаваемые параметры: указатель на функцию, задержка перед 1-м запуском, период запуска(0-однократный запуск)
// Время выдержки в тиках системного таймера. Возвращет код ошибки.
//===============================================================================================
U_ALU_INT SetTimerTask(T_PTR TS, unsigned int Prolongation, unsigned int NewPeriod)    //1 task ~12words
{
U_ALU_INT	index = 0;
U_ALU_INT	result = QUEUE_FULL;
 ATOMIC_BLOCK_FORCEON
 {
	 FOREACH_TASK	// Поиск следующей доступной позиции в массиве задач TODO: хорошо бы избавиться от перебора, а использовать переход по индексу в таблице (возможно переписать в switch TS case index)
  {
	if (TTask[index].GoToTask == TS)			// ищем заданый таймер
	{   // Если задача не помечена как мёртвая(зависшая) утилитой KERNEL_CorpseService()...
          if((TTask[index].TStatus != DEAD) && (TTask[index].TStatus != DELAYED) && (TTask[index].TStatus != SYSPENDED))
	    { //..и если задача на задержке - перезаписывать статус НЕЛЬЗЯ иначе будет переполнение стека!!!
			TTask[index].TDelay  = Prolongation;		// И поле выдержки времени
			TTask[index].TPeriod = NewPeriod;	    // И поле периода запуска
			TTask[index].TStatus = WAIT;             // Флаг - ожидает выполнения!
			result = TASK_REWRITTEN;
                        NEED_TO_SORT = 1; //разрешить сортировку задач!
                        goto exit;			// Выход.
	    }	else{result = DEAD_TASK; goto exit;}		//Устанавливать на выполнение висячие задачи нельзя!
	  }
  }
    // Если не находим - значит она новая
	if(timers_cnt_tail < TASK_QUEUE_SIZE)// И в очереди есть место - добавляем задачу в конец очереди
	{
			TTask[timers_cnt_tail].GoToTask   = TS;			    // Заполняем поле перехода задачи
			TTask[timers_cnt_tail].TDelay  = Prolongation;   // И поле выдержки времени
			TTask[timers_cnt_tail].TPeriod = NewPeriod;	    // И поле периода запуска
			TTask[timers_cnt_tail].TStatus = WAIT;           // Флаг - ожидает выполнения!
			timers_cnt_tail++;                          		// Увеличиваем кол-во (новых) таймеров
			result = TASK_ADDED;
                        NEED_TO_SORT = 1; //разрешить сортировку задач!
                        goto exit;			    	// Выход.
	}
exit: ;
 }//end of  ATOMIC_BLOCK_
return result; // return c кодом ошибки - нет свободных таймеров, таймер перезаписан или добавлен как новый
}
//===============================================================================================

/*
Служба таймеров ядра. Должна вызываться из прерывания раз в 1мс. Хотя время можно варьировать в зависимости от задачи

TODO: Привести к возможности загружать произвольную очередь таймеров. Тогда можно будет создавать их целую прорву.
А также использовать эту функцию произвольным образом.В этом случае не забыть добавить проверку прерывания.
*/
//===============================================================================================
inline void TimerService(void)
{
U_ALU_INT index = 0;
 ATOMIC_BLOCK_FORCEON
 {
   while(index < timers_cnt_tail)	// Прочесываем очередь таймеров
   {																		//Зависшие задачи никогда не станут выполняться
      switch (TTask[index].TStatus) //if((TTask[index].TStatus == WAIT) || (TTask[index].TStatus == DONE))// Если не выполнилась или выполнилась
      {
				case WAIT: //Ждёт запуска
				case DONE: //Отработала
				case RDY:  //Если задача вовремя не выполнилась

             if(TTask[index].TDelay > 0)  // таймер не выщелкал, то
              {
                TTask[index].TDelay--;	 // щелкаем еще раз.
              }
              else                           //Ставим флаг готовности к выполнению
              {
								TTask[index].TStatus = RDY;
								TASK_RDY_TO_DISPATCH = 1;   //Можно запускать диспетчер и прочёсывать очередь, задача установлена!
               #ifdef USE_TTASKS_LOGGING
                TTask[index].run_me_cnt++;      //пора запустить задачу ++ раз
               #endif
              }
        break;  //WAIT or DONE or RDY
      default: break;
      }
    index++;
    }
  }
}
//===============================================================================================


//===============================================================================================
void ClearTimerTask(T_PTR TS)  //обнуление таймера, очистка задачи
{
uint8_t	 	index=0;
ATOMIC_BLOCK_FORCEON
 {
   FOREACH_TASK
    {
      if(TTask[index].GoToTask == TS)
      {
           if(index < (timers_cnt_tail))         // переносим последнюю задачу
         {
            TTask[index] = TTask[timers_cnt_tail - 1];  // на место удаляемой
            TaskSetToNULL(timers_cnt_tail - 1);//зануление последней задачи (ДЛЯ ЭКОНОМИИ ВРЕМЕНИ - МОЖНО НЕ ЗАНУЛЯТЬ!)
         }
           else//Если задача последняя в очереди
         {
            TaskSetToNULL(index);
         }
        --timers_cnt_tail;  //уменьшаем кол-во задач
        NEED_TO_SORT = 1; //разрешить сортировку задач!
        goto exit;//exit of for-cycle
      }
    }//end FOREACH_TASK
exit: ;
 }//end of  ATOMIC_BLOCK_
}
//===============================================================================================

uint8_t TaskExist(T_PTR TS)
{
	uint8_t state = 0;
	uint8_t	 	index=0;
	
ATOMIC_BLOCK_FORCEON
 {
   FOREACH_TASK
    {
      if(TTask[index].GoToTask == TS)
      {
				state = 1;
				break;
			}
		}
	}
	return  state;
}

//===============================================================================================
#ifdef USE_SORTING_TTASK_QUEUE
#warning добавить ещё сортировку по времени выполнения и подравнивание времени выполнения, если оно съезжает!!!!!!!!
#warning добавить динамическую перенастройку rtos таймера на пробуждение проца только ко времени выполнения след. задачи, а не каждый тик!!!!!!!!
 inline void KERNEL_Sort_TaskQueue (void) //сортировкa задач по периоду выполнения (наиболее частые - ближе к началу очереди!)
 {
  TASK_STRUCT 	tmp;
  int8_t 		l, r, k, index;

 ATOMIC_BLOCK_FORCEON
 {  
   //+++++++++++++  //Шейкерная сортировка
           k = l = 0;
           r = timers_cnt_tail - 2; //
           while(l <= r)
           {
              for(index = l; index <= r; index++)
              {
                 if (TTask[index].TPeriod > TTask[index+1].TPeriod)
                 {
                 tmp = TTask[index];   TTask[index] = TTask[index+1]; // swap
                 TTask[index+1] = tmp; k = index;
                 }
              }
              r = k - 1;

              for(index = r; index >= l; index--)
               {
                 if (TTask[index].TPeriod > TTask[index+1].TPeriod)
                 {
                 tmp = TTask[index];   TTask[index] = TTask[index+1]; // swap
                 TTask[index+1] = tmp; k = index;
                 }
               }
              l = k + 1;
           }
 //-------------
 }//end of  ATOMIC_BLOCK_
 }
 #endif
 //===============================================================================================

#warning TMP fix
 //===============================================================================================
 //#ifdef USE_CORPSE_SERVISE
 //===============================================================================================
 U_ALU_INT SetTaskDeadtime(T_PTR TS, uint8_t DeadTime) //DeadTime = 0xFF means DeadTimer (for this task) is OFF, DeadTime = 0x00 use default deadtime
 {
	  #ifdef USE_CORPSE_SERVISE
 U_ALU_INT	index = 0;
 U_ALU_INT	result = QUEUE_FULL;

  ATOMIC_BLOCK_FORCEON
  {
   FOREACH_TASK
   {
 	if (TTask[index].GoToTask == TS)			// ищем заданый таймер
 	{
 		if(TTask[index].TStatus != DEAD)		 // Если задача не помечена как мёртвая(зависшая) утилитой KERNEL_CorpseService()
 		{
 			TTask[index].TDeadtime  = DeadTime;		    // И поле выдержки времени
 			result = TASK_REWRITTEN; goto exit;	 // Выход.
 		}	else{result = DEAD_TASK; goto exit;}		//Устанавливать на выполнение висячие задачи нельзя!
 	  }
   }
 exit: ;
  }//end of  ATOMIC_BLOCK_
   return result; // return c кодом ошибки - нет свободных таймеров, таймер перезаписан
	#endif
 }
 
 //===============================================================================================
 #ifdef USE_CORPSE_SERVISE
 //===============================================================================================
 inline void KERNEL_CorpseService(void) //Обработка зависших задач
{
 static T_PTR 		DeadTask_prev, DeadTask_curr;
 static uint16_t 	Timeout_delay = 0;
 static uint8_t 	coins = 0; //совпадения
 uint8_t			index = 0;
 uint8_t			DeadTaskLocalTimeout = 0;
 static bit 		suspect_flag = 0;
	
	TASK_STRUCT tsHndl;
	
 ATOMIC_BLOCK_FORCEON
 {
  if(InfiniteLoopFlag == 0) //диспетчер сбросил флаг, значит задача завершилась
  {
	goto EXIT;
  }
  else  //флаг не сброшен - выполняется какая-то задача! возможно уже долго или вообще зависла
  {
	  FOREACH_TASK	//поиск мертвеца (пока ещё просто возможного!)
			 {
				if((TTask[index].TStatus == IN_PROC) && (TTask[index].TDeadtime !=0xFF)) //фильтр задач на игнорирование
				{
					if(suspect_flag == 0)//при первом заходе
					{
						DeadTask_prev = TTask[index].GoToTask;
						suspect_flag = 1; //начинаем подозревать
						Timeout_delay = (uint16_t)v_u32_SYS_TICK; //засекли таймаут
						goto ATOMIC_EXIT;
					}
					else //при втором заходе
					{
						DeadTask_curr = TTask[index].GoToTask;
						if(DeadTask_curr == DeadTask_prev)	//подозрения подтвердились
							{
							    coins++;
								if(TTask[index].TDeadtime==0){DeadTaskLocalTimeout = DeadTaskDefaultTimeout;}
								else{DeadTaskLocalTimeout = TTask[index].TDeadtime;}

								if((v_u32_SYS_TICK - Timeout_delay >= DeadTaskLocalTimeout)&&(coins>=4))
								{
                   TTask[index].TStatus = DEAD;	//Поставить метку (обработать в будущем)
									
									USER_DeadTaskHook(TTask[index].GoToTask); // Вызов пользовательского обработчика исключений. В параметре - возможно(!) зависшая задача.
									
								  //ClearTimerTask(DeadTask_curr);	//или просто выпилить из очереди!
									//TODO: нужно куда-то сообщать в этом случае. Хотя бы в дебаге!

								  InfiniteLoopFlag = 1; //Установка до след. интерации
                                  suspect_flag = 0;Timeout_delay = 0;
							      DeadTask_curr = DeadTask_prev = 0; coins = 0;
								  //на goto EXIT; нельзя т.к.
                                   //TODO: Теперь надо передать управление системе (пока не реализовано) //setjmp/longjmp в помощь!

									//FLAG_SET(g_tcf,DEAD_TASK_DELETED);
									//TaskManager();
									//#asm("JMP 0x0000");
									// #asm("call TaskManager");
									//goto DEAD_TASK_DETECTED;
								}
							}
							else //подозревания НЕ подтвердились (На выполнении сейчас другая задача)
							{
EXIT:
									InfiniteLoopFlag = 1; //Установка до след. интерации и выход
									suspect_flag = 0;Timeout_delay = 0;
									DeadTask_curr = DeadTask_prev = 0; coins = 0;
							}

					}
				}
			 }//end FOREACH_TASK
  }
ATOMIC_EXIT: ;
   } //end of  ATOMIC_BLOCK_
}
#endif
//===============================================================================================

//TODO: дописать и расширить!
//===============================================================================================
#ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
inline void KERNEL_TaskRetHandler(T_PTR task, T_RET val)
{
char tmp_str[10];
itoa((int)task,tmp_str);                //for ret/arg tests
Put_In_Log("Task: ");Put_In_Log(tmp_str);Put_In_Log("\r"); //for ret/arg tests
itoa(val,tmp_str);
Put_In_Log("RET = ");Put_In_Log(tmp_str);Put_In_Log("\r"); //for ret/arg tests
}
#endif
//===============================================================================================

//===============================================================================================
 void ProcessTasksMessages(void)
{
  #ifdef USE_USER_HOOKS
USER_TaskFlagsHandler();
  #endif
//Заготовка для обработчика сообщений от процессов
}
//===============================================================================================

//===============================================================================================
#ifdef USE_MAIL_BOXES

char* MessagePack(char *str, uint16_t len)
{
char *value;
  value = (char *) malloc (len + 1);
  if(!value){/*fatal ("virtual memory exhausted");*/ return 0;}
  value[len] = '\0';
  return (char*) memcpy (value, str, len);
}

uint16_t MessageSend(T_PTR Reciever, MESSAGE_T *Msg)
{
uint16_t	index = 0;
uint16_t	result = TASK_NOT_FOUND;

 ATOMIC_BLOCK_FORCEON
 {
  FOREACH_TASK
  {
	if (TTask[index].GoToTask == Reciever)	//Reciever exist
	{
		if(TTask[index].TStatus != DEAD)
		{
			TTask[index].Sender  = Msg->Sender;
			TTask[index].Message = Msg->Message;
			TTask[index].Size = Msg->Size;
            TTask[index].ActionFlags = Msg->ActionFlags;
			result = TASK_SEND_MESSAGE;
		}	else{result = DEAD_TASK;}
	}
  }
 } //end ATOMIC_BLOCK_
 return result;
}

uint16_t MessageGet(T_PTR Reciever, MESSAGE_T *Msg)
{
uint16_t* p;
uint16_t size;
uint16_t Err = 0;
uint16_t	index = 0;
uint16_t	result = TASK_NOT_FOUND;

 ATOMIC_BLOCK_FORCEON
 {
  FOREACH_TASK
  {
	if (TTask[index].GoToTask == Reciever)
	{
		if(TTask[index].TStatus != DEAD)
		{
			Msg->Sender = TTask[index].Sender;
			Msg->Message = TTask[index].Message;
			Msg->Size = TTask[index].Size;
              if(TTask[index].ActionFlags == 0)
             {
			    TTask[index].Sender = 0;
			    TTask[index].Message = 0;
			    TTask[index].Size = 0;
             }
            result = TASK_GOT_MESSAGE;
		}	else{result = DEAD_TASK;}
	}
  }
 } //end ATOMIC_BLOCK_
 return result;
}
#endif
//===============================================================================================


 //===============================================================================================
 //Очистка очереди от дубликатов задач с разным временем
 void clear_duplicates (void) //not tested
 {
  uint8_t		index=0;
  bit		nointerrupted = 0;
  T_PTR task_src;
  ATOMIC_BLOCK_FORCEON
  {
	  FOREACH_TASK
	  {
		 task_src = TTask[index].GoToTask;
		 for(index+1;index!=timers_cnt_tail;++index)
		  {
		   if (TTask[index].GoToTask == task_src) {TTask[index].GoToTask = Idle;}
		  }
	  }
  }// Разрешаем прерывания
 }
//===============================================================================================
/*
 *	                    T_PTR 	 GoToTask; 		// Указатель перехода

                        uint16_t TDelay;		// Выдержка в мс перед стартом задачи
                        uint16_t TPeriod;	// Выдержка в мс перед следующим запуском
                        uint8_t  TStatus;
#ifdef USE_CORPSE_SERVISE
                        uint16_t TDeadtime;       // Время на выполнение задачи 0 - дефолтный deadtime, 0xFF - бесконечно
#endif

#ifdef USE_TTASKS_LOGGING
                        char*    TName;
                        uint16_t run_me_cnt;     //кол-во (возможно) упущеных запусков задачи
                        uint32_t exec_time;      // Реально замеряное время выполнения задачи
                        uint32_t sys_tick_time;  // Значение системного таймера на момент запуска задачи в тиках
                        uint32_t work_time_min;		//мин Время выполнения задачи
                        uint32_t work_time_max;		//макс Время выполнения задачи
#endif
#ifdef USE_MAIL_BOXES
                        T_PTR    Sender;
                        void*    Message;
                        uint16_t Size;
                        uint16_t ActionFlags;
#endif
 */
//===============================================================================================
 void TaskSetToNULL(uint16_t index)
 {
				TTask[index].GoToTask = Idle;
				TTask[index].TDelay = 0; 		// Обнуляем время
				TTask[index].TPeriod = 0; 		// Обнуляем время
				TTask[index].TStatus = WAIT; 	// Обнуляем status
#ifdef USE_CORPSE_SERVISE
				TTask[index].TDeadtime = 0xFF;  // Время на выполнение задачи 0 - дефолтный deadtime, 0xFF - бесконечно
#endif
#ifdef USE_TTASKS_LOGGING
        TTask[index].TName = NULL;
        TTask[index].run_me_cnt = 0;// Кол-во пропущеных запусков
				TTask[index].exec_time = 0; // Время выполнения
        TTask[index].sys_tick_time = 0;// Системное время последнего запуска
        TTask[index].work_time_max = 0;
        TTask[index].work_time_min = 5000000;
#endif
#ifdef USE_MAIL_BOXES
        TTask[index].Sender = NULL;
        TTask[index]. Message = NULL;
        TTask[index].Size = 0;
        TTask[index].ActionFlags = 0;
#endif
 }
//===============================================================================================
 //TODO: реализовать задержку!
#warning в задержку добавить проверку вызова из прерывания типа OSIntNesting !!!!
/*
 void  OSTimeDly (INT32U ticks)
 {
     INT8U      y;
     if (OSIntNesting > 0u) {                     // See if trying to call from an ISR
         return;
     }
     if (OSLockNesting > 0u) {                    // See if called with scheduler locked
         return;
     }
     if (ticks > 0u) {                            // 0 means no delay!
         OS_ENTER_CRITICAL();
         y            =  OSTCBCur->OSTCBY;        //Delay current task
         OSRdyTbl[y] &= (OS_PRIO)~OSTCBCur->OSTCBBitX;
         if (OSRdyTbl[y] == 0u) {
             OSRdyGrp &= (OS_PRIO)~OSTCBCur->OSTCBBitY;
         }
         OSTCBCur->OSTCBDly = ticks;              // Load ticks in TCB
         OS_EXIT_CRITICAL();
         OS_Sched();                              // Find next task to run!
     }
 }
 */
		#ifdef USE_TTASKS_LOGGING
 //===============================================================================================
  inline void KERNEL_Get_TaskStats (uint16_t index)
  {

  char tmp_str[65];
  FullStopRTOS();
  	ATOMIC_BLOCK_FORCEON
  	{
          Put_In_Log("\rIdle_statts_cnt: ");
         itoa(Idle_statts_cnt, tmp_str);
         Put_In_Log(tmp_str);

  	 Put_In_Log("\r<");
  		 itoa((int)TTask[index].GoToTask , tmp_str);
  		 Put_In_Log(tmp_str); Put_In_Log(",");
  		 itoa((int)TTask[index].TDelay , tmp_str);
  		 Put_In_Log(tmp_str); Put_In_Log(",");
  		 itoa((int)TTask[index].TPeriod , tmp_str);
  		 Put_In_Log(tmp_str); Put_In_Log(",");
  		 itoa((int)TTask[index].sys_tick_time , tmp_str);
  		 Put_In_Log(tmp_str); Put_In_Log(",");
  		 itoa((int)TTask[index].exec_time , tmp_str);
  		 Put_In_Log(tmp_str);Put_In_Log(",");
         itoa((int)TTask[index].work_time_max , tmp_str);
  		 Put_In_Log(tmp_str);Put_In_Log(",");
  		 itoa((int)TTask[index].TStatus , tmp_str);
  		 Put_In_Log(tmp_str);Put_In_Log(",");
  		 itoa((int)TTask[index].run_me_cnt , tmp_str);
  		 Put_In_Log(tmp_str);
  	Put_In_Log(">\r");
  	}
  RunRTOS();

  }

//===============================================================================================

// ====================================================================================
uint32_t CalkElapsed(uint32_t start, uint32_t stop)
{
uint32_t cycle_time_elapsed  = 0;
		  if(stop >= start)  { cycle_time_elapsed = stop - start;  }
		  else { cycle_time_elapsed = start - stop; }
return 	cycle_time_elapsed;
}

inline void HandlerProfiling_Start(void)
{
 TMP_CurrentTask = Shed_CurrentTask; // ПРИМ - интересует статистика по выполненой задаче
 cycle_time_start = GET_TSC;//SysTick->VAL;
}

// ====================================================================================
//#define FOREACH_TASK 	for(index;index<timers_cnt_tail;++index)

#warning wtf with inline in KEIL_5???
/*inline*/ void HandlerProfiling_End(void)
{
uint32_t index = 0;
static uint32_t Skip = 0;
uint32_t CurrDuration = 0;
uint32_t MaxDuration = 0;
uint32_t MinDuration = 0xFFFFFFF0;


 if((TMP_CurrentTask != NULL) &&(Idle_statts_cnt == 0))
	{
		if(Skip < 100)// Пропуска накопления статистики, для первых ста вхождений
		{
			Skip++;
			return;
		}

	 ATOMIC_BLOCK_FORCEON
	 {
		cycle_time_end = GET_TSC;//SysTick->VAL;
		CurrDuration = CalkElapsed(cycle_time_start, cycle_time_end);
		CurrDuration = CurrDuration/72; //дел на 72, чтобы получить время в мкС
		
		 FOREACH_TASK
		  {
			 if(TTask[index].GoToTask == TMP_CurrentTask)
			 {
				 MaxDuration = TTask[index].work_time_max;
				 MinDuration = TTask[index].work_time_min;

					//Захват пиковых значений
						   //MaxDuration = (CurrDuration + MaxDuration)/2;
						 if(CurrDuration >= MaxDuration){ TTask[index].work_time_max = CurrDuration;}
				 if(MinDuration >= CurrDuration){ TTask[index].work_time_min = CurrDuration;}
			break;
			 }
		  } //end FOREACH_TASK	
		TMP_CurrentTask = NULL;
	 } //end ATOMIC_BLOCK_FORCEON
	}
}


 //===============================================================================================
// Присвоение названия задаче (для отладки) 
 void TaskName_Assign (T_PTR task, char* name)
 {
 uint16_t index = 0;

  ATOMIC_BLOCK_FORCEON
  {
  FOREACH_TASK
	{
		if(TTask[index].GoToTask == task)
		{
		TTask[index].TName = name;
		break;
		}
	}
  }
 }

#endif// USE_TTASKS_LOGGING

//Дебажные выводы
 //===============================================================================================
#ifdef DEBUG
 #ifdef USE_TTASKS_LOGGING
DECLARE_TASK (Task_t_props_out)
{
uint8_t index = 0;
char tmp_str[20];

 FullStopRTOS();
    // LED_PORT  &=~(1<<LED2);
 FOREACH_TASK
	{
     Put_In_Log("\r\n<");
     itoa((int)TTask[index].GoToTask , tmp_str);
     Put_In_Log(tmp_str); Put_In_Log(",");
     itoa((int)TTask[index].TDelay , tmp_str);
     Put_In_Log(tmp_str); Put_In_Log(",");
     itoa((int)TTask[index].TPeriod , tmp_str);
     Put_In_Log(tmp_str); Put_In_Log(",");
     itoa((int)TTask[index].sys_tick_time , tmp_str);
     Put_In_Log(tmp_str); Put_In_Log(",");
     itoa((int)TTask[index].exec_time , tmp_str);
     Put_In_Log(tmp_str);Put_In_Log(",");
     itoa((int)TTask[index].TStatus , tmp_str);
     Put_In_Log(tmp_str);Put_In_Log(",");
     itoa((int)TTask[index].run_me_cnt , tmp_str);
     Put_In_Log(tmp_str);
     Put_In_Log(">");
  }
 // LED_PORT  |=(1<<LED2);
 Put_In_Log("\r\n");
 _irqEn();
 Task_LogOut(0);
RunRTOS();
}
	#endif
#endif

#ifdef USE_USER_HOOKS
 void USER_IdleHook(){}          //Used in Idle process
 void USER_RtosTimerIsrHook(){}  //Used in Rtos timer ISR
 void USER_TaskFlagsHandler(){}  //Used in Idle process
 //void USER_DeadTaskHook(T_PTR task){}  		//Used in KERNEL_CorpseService when task ready to mark as DEAD
	
#else //empty hooks
 void USER_IdleHook(){}          //Used in Idle process
 void USER_RtosTimerIsrHook(){}  //Used in Rtos timer ISR
 void USER_TaskFlagsHandler(){}  //Used in Idle process
 void USER_DeadTaskHook(T_PTR task){}  		//Used in KERNEL_CorpseService when task ready to mark as DEAD
#endif
/*
T_PTR GoToTask; 					// Указатель перехода
uint16_t TaskDelay;				// Выдержка в мс перед старотом задачи
uint16_t TaskPeriod;			// Выдержка в мс перед следующим запуском
uint8_t TaskStatus;
  uint32_t sys_tick_time; // Значение системного таймера на момент выполнения задачи в тиках
  uint8_t exec_time;       // Реально замеряное время выполнения задачи
  uint8_t hndl;
*/

//T_PTR> Delay> Period> tick> exec Status>

/*2400
<3720,8,10,2404,0,0>
<4295,0,0,0,0,3>
<3759,0,33,0,0,1>
<3674,107,500,2013,46,0>
<3835,48,50,2404,1,0>
<3882,33,333,2106,0,0>
<3666,0,10,2363,0,1>
<3673,0,250,2148,1,1>
<3663,97,0,0,0,0>
<3727,0,3,2363,41,1>
+ */

//Возможно сделась задачу с параметром и возвращаемым значением! +

/*Возможно стоит вести отсчёт пропущенных (RunMe) запусков так же void scheduler_update(void) interrupt
{
    foreach (task in all_task_list)
    {
        task.PeriodCur--;
        if (task.PeriodCur == 0)
        {
            task.PeriodCur = task.Period;
            task.RunMe++;
        }
    }
}
void dispatch_tasks(void)
{
    foreach (task in all_task_list)
    {
        if (task.RunMe > 0)
        {
            task.pTask();
            task.RunMe--;
        }
    }
}
*/
