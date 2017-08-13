#ifndef EERTOS_DEBUG_H
#define EERTOS_DEBUG_H

#include "..\platform.h"

//#define _LED_B_ON  STM_EVAL_LEDOn(LED3); //LED_PORT|=(1<<LED1)
//#define _LED_B_OFF STM_EVAL_LEDOff(LED3); //LED_PORT&=~(1<<LED1)

#define LED_Port GPIOB
#define LED_B_Pin GPIO_Pin_13

#define _LED_B_ON  GPIO_ResetBits(LED_Port, LED_B_Pin)
#define _LED_B_OFF GPIO_SetBits(LED_Port, LED_B_Pin)

#define LogBufSize 3064 //Размер буффера для логов


void WorkLogPutChar(unsigned char symbol);
void Put_In_Log (unsigned char * data);
void LogOut(void);


//получить размер памяти, статически выделеной под задачи
uint32_t Get_StaticTasksRam(uint16_t amount_of_tasks);

#endif
