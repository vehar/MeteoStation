#ifndef EERTOS_H
#define EERTOS_H

#include "EERTOSHAL.h"

//#define USE_MINIMAL_DESIGN 

//#define USE_FULL_DESIGN

#define USE_CUST_DESIGN

//#define DEBUG

////////////////////////////////////////////////////////////////////////////////////////////
//Custom RTOS design
//==========================================================================================
#define USE_SORTING_TTASK_QUEUE

//#define USE_MAIL_BOXES

//#define USE_CORPSE_SERVISE 			//TODO: fix os if not defined! //Delete dead_timeout_tasks from queue

//#define USE_TTASKS_LOGGING			

//#define USE_UINT16_T_TASK_VS_T_ARG_PARAMETER

#define USE_VOID_TASK_VS_VOID_PARAMETER

#define USE_USER_HOOKS
//==========================================================================================

#ifdef USE_FULL_DESIGN

    #warning USED_FULL_DESIGN

	#define USE_SORTING_TTASK_QUEUE

    #define USE_MAIL_BOXES

	#define USE_CORPSE_SERVISE 	//TODO: fix os if not defined!		//Delete dead_timeout_tasks from queue

	#define USE_TTASKS_LOGGING //TODO: fix os if not defined!

	#define USE_UINT16_T_TASK_VS_T_ARG_PARAMETER

    //#define USE_USER_HOOKS

#elif USE_MINIMAL_DESIGN

    #warning USED_MINIMAL_DESIGN

	#define USE_VOID_TASK_VS_VOID_PARAMETER

#else

	#ifndef USE_CUST_DESIGN
    #error Конфигурация системы не указана!
    #endif

#endif

#ifdef DEBUG
  #warning !!!!!!------you should use "USE_UINT16_T_TASK_VS_T_ARG_PARAMETER" define ----!!!!!!
	#warning !!!!!!------you should use "USE_TTASKS_LOGGING" define ----!!!!!!
#endif

#define LONG_IDLE_PERIOD 			5000
#define TASK_QUEUE_SIZE				30
#define QUEUE_SORTING_PERIOD 		100 //ticks(!)  //можно увеличить, чтоб не грузить Idle-задачу
#define GLOBAL_DELAY_DEFTH_LIMIT	5 //кол-во вложеных пауз

#define FOREACH_TASK 				for(index;index<timers_cnt_tail;++index)

#ifdef USE_VOID_TASK_VS_VOID_PARAMETER
	typedef void (*T_PTR)(void);
    #define DECLARE_TASK(t_name)         void t_name (void)
	#define return_val(x)         		 return
#endif

#ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
	typedef void* T_ARG;
    typedef uint16_t T_RET;
	typedef T_RET (*T_PTR)(T_ARG);
    #define  DECLARE_TASK(t_name)        T_RET t_name (T_ARG T_ARG_VAL /*, char* name = "##t_name"*/)
	#define return_val(x)         		 return (x)
#endif

#ifdef  USE_MAIL_BOXES
typedef  struct {
            void* Message;
            uint16_t Size;
            T_PTR Sender;
            uint16_t ActionFlags; //what to do with resieved message: 0 - del, 1 - save
}MESSAGE_T;
/*Message Interface Example
DECLARE_TASK(Malloc_Test)
{
char* p;                                                     //ptr to message arr
uint16_t result;
MESSAGE_T  Malloc_Test_Message;                              //message struct
TASK_ARG_TEST("Malloc_Test\n");

 p = MessagePack("Ololol_55 =)",500);                        //cpy string to mess arr  (may be str or arr of int-s)
 if(!p){result = 0;goto exit;}                               //Used malloc, so ptr need tp be tested

 Malloc_Test_Message.Message = p;                            //place mess and attributes to struct
 Malloc_Test_Message.Size = 500;
 Malloc_Test_Message.Sender = Malloc_Test;
 Malloc_Test_Message.ActionFlags = 1;

 result = MessageSend(Malloc_Test_RX, &Malloc_Test_Message); //send struct
 if(result != TASK_SEND_MESSAGE){free(p);}                   //Used malloc, so ptr need tp be tested

 //SetTask(Malloc_Test_RX); //Решить проблемму записи в однократные задачи!
 exit:
return result;
}

DECLARE_TASK(Malloc_Test_RX)
{
char  *p;                                                             //ptr to message arr
uint16_t size;
uint16_t result;
uint16_t err;
uint16_t flags;
MESSAGE_T  Malloc_Test_RX_Message;
TASK_ARG_TEST("Malloc_Test_RX\n");

result = MessageGet(Malloc_Test_RX, &Malloc_Test_RX_Message);
p = Malloc_Test_RX_Message.Message;                                 //Get mess string
Put_In_Log(p);
size = Malloc_Test_RX_Message.Size;
flags = Malloc_Test_RX_Message.ActionFlags;
if(result == TASK_GOT_MESSAGE){free(p);}                            //Free memory
err = size;
return  err;
}
*/
#endif

enum TASK_STATUS {WAIT, RDY, IN_PROC, DONE, DEAD, DELAYED, SYSPENDED};

enum SetTimerTask_Status {OK, QUEUE_FULL, TASK_REWRITTEN, TASK_ADDED, DEAD_TASK, TASK_SEND_MESSAGE, TASK_GOT_MESSAGE, TASK_NOT_FOUND};

/*
uint16_t g_tcf = 0; //_global_taskControl_flags
 enum g_tcf_values {
                      ERR_UNDEF_FLAG, //ошибка - неопределённый флаг
                      S_SPI_BUF_CLR,
                      FLUSH_WORKLOG,
                      DEAD_TASK_DELETED
 };
 */


#warning оптимизировать передачей указателя или ссылки на структуру
typedef  struct
{
	                    T_PTR 	 GoToTask; 		// Указатель перехода

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
}TASK_STRUCT;// Структура программного таймера-задачи


//+++++++++++++PRIVATE RTOS SERVICES++++++++++++++++++++++++++++++
//change extern-s to static
static DECLARE_TASK (Idle);
static void  TaskManager(void);
inline static void TimerService(void);
static void ProcessTasksMessages(void);      //Обработчик флагов и сообщений задач
inline static void ContextSave(void);    //TODO
inline static void ContextRestore(void); //TODO
#ifdef USE_CORPSE_SERVISE
inline static void KERNEL_CorpseService(void);
#endif
#ifdef USE_SORTING_TTASK_QUEUE
inline static void KERNEL_Sort_TaskQueue (void);
#endif
#ifdef USE_UINT16_T_TASK_VS_T_ARG_PARAMETER
inline static void KERNEL_TaskRetHandler(T_PTR task, T_RET val); //Обработка кодов выхода задачи
#endif
#ifdef USE_TTASKS_LOGGING
void static KERNEL_Get_TaskStats(uint16_t index); //Получение статистики задачи
#endif
#ifdef DEBUG
DECLARE_TASK (Task_t_props_out);
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++PUBLIC RTOS SERVICES++++++++++++++++++++++++++++++++
//------------Main controls-----------------------------------------
extern void InitRTOS(void);//Инициализация RTOS
extern void RunRTOS(void);//Старт ядра
extern void SuspendRTOS(void);//Приостановка ядра, Фактически снижение частоты системного таймера
extern void FullStopRTOS(void);//Остановка системного таймера
//-------------------------------------------------------------------
//В прерывание по RTOS_TIMER TICK!
//void RTOS_TIMER_ISR_HNDL(void);

//Бесконечный цикл обработки задач (после всей инициализации)
extern void Shedull (uint8_t param);

//Поставить задачу в очередь на выполнение с задержкой в Prolongation и периодом NewPeriod и игнорированием долгого выполнения
extern void SetTimerTaskInfin(T_PTR TS, unsigned int Prolongation, unsigned int NewPeriod); 
	
//Поставить задачу в очередь на немедленное выполнение
extern void SetTask(T_PTR TS);

//Поставить задачу на паузу
extern void TaskSuspend(T_PTR TS);

//Продолжить выполнение
extern void TaskResume(T_PTR TS);

//Существует ли задача
extern uint8_t TaskExist(T_PTR TS);

//Поставить задачу в очередь на выполнение с задержкой в Prolongation и периодом NewPeriod
extern U_ALU_INT SetTimerTask(T_PTR TS, unsigned int Prolongation, unsigned int NewPeriod);

#ifdef USE_TTASKS_LOGGING
//Назначить имя задаче (для отладки)
extern  void TaskName_Assign (T_PTR task, char* name);
#endif

//Удалить задачу из очереди
extern void ClearTimerTask(T_PTR TS);

extern void T_Delay(uint32_t delay_ticks); //Задержка внутри задачи

#warning TMP fix
//#ifdef USE_CORPSE_SERVISE
//Установить время, по истечении которого задача считается зависшей и удаляется из очереди
extern U_ALU_INT SetTaskDeadtime(T_PTR TS, uint8_t DeadTime);
//#endif

#ifdef USE_USER_HOOKS
extern void USER_IdleHook();          //Used in Idle process
extern void USER_RtosTimerIsrHook();  //Used in Rtos timer ISR
extern void USER_TaskFlagsHandler();  //Used in Idle process
extern void USER_DeadTaskHook(T_PTR task);  		//Used in KERNEL_CorpseService when task marked as DEAD
#endif

#ifdef  USE_MAIL_BOXES
char* MessagePack(char* str, uint16_t len);
extern uint16_t MessageSend (T_PTR Reciever, MESSAGE_T *Msg);
extern uint16_t MessageGet(T_PTR Reciever, MESSAGE_T *Msg);
#endif
 extern void RTOS_TIMER_ISR_HNDL(void); //Pun in Rtos timer ISR!!
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 #warning TODO исправить странное зависание и отсутствие вывода при SetTimerTask(Task_Flush_WorkLog,<55,0); в LodOut!
           //SetTimerTask не работает из другой задачи!

 #warning !!!!!! Если в задаче Host устанавливается задача Slave, то при периоде Host<Slave Slave всё время будет затираться и никогда не выполнится!


#ifdef USE_CORPSE_SERVISE
volatile static uint16_t DeadTaskDefaultTimeout = 10;
extern  volatile bit InfiniteLoopFlag;
extern  volatile uint16_t DeadTaskDefaultTimeout;
#endif


 extern  volatile uint32_t v_u32_SYS_TICK;

#ifdef DEBUG
 extern  volatile uint8_t v_u8_SYS_TICK_TMP1;
 extern void Put_In_Log (unsigned char * data);
 extern DECLARE_TASK(Task_LogOut);
#endif

//RTOS Errors Пока не используются.
#define TaskSetOk			 'A'
#define TaskQueueOverflow	 'B'
#define TimerUpdated		 'C'
#define TimerSetOk			 'D'
#define TimerOverflow		 'E'

#endif
