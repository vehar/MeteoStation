#include "EERTOS.h"
#include "EERTOSHAL.h"
#include "EERTOS_DEBUG.h"

 char WorkLog[LogBufSize];
 volatile uint16_t LogIndex = 0;
 
void  SendStr(char* buff)
{
	#warning define it!
}


/////// Отладочный кусок.
//Вывод лога работы конечного автомата в буфер памяти, а потом.
//По окончании работы через UART на волю

void WorkLogPutChar(unsigned char symbol)
{
ATOMIC_BLOCK_FORCEON
	{
		if (LogIndex <LogBufSize-10)            // Если лог не переполнен
		{
				WorkLog[LogIndex]= symbol;    // Пишем статус в лог
				LogIndex++;
		}
	}
}

//#warning  -добавить после каждого слова прибавление \r\n для єкономии РАМ
void Put_In_Log (unsigned char * data)
{
  while(*data)
  {
    WorkLogPutChar(*data++);
  }
  /// WorkLogPutChar(10);//\r
   //WorkLogPutChar(13);//\n
}

extern void SendStr(char *Str);
void LogOut(void)				// Выброс логов
{
SuspendRTOS();

  ATOMIC_BLOCK_FORCEON
  {
    if(LogIndex < LogBufSize - 2)
    {
    WorkLog[LogIndex]= '+';
    LogIndex++;
    }
    //USART_Send_Str(SYSTEM_USART, WorkLog);
    SendStr(WorkLog);
    while(LogIndex){WorkLog[LogIndex] = 0; LogIndex--;};
  }
RunRTOS();
//LogIndex = 0;
//FLAG_SET(g_tcf, FLUSH_WORKLOG);
//SetTimerTask(Task_Flush_WorkLog,25,0);//очистка лог буффера
}



//получить размер памяти, статически выделеной под задачи
uint32_t Get_StaticTasksRam(uint16_t amount_of_tasks)
{
return sizeof(TASK_STRUCT)*amount_of_tasks;
}
