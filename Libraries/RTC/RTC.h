#ifndef __RTC_H
#define __RTC_H

#include "stdint.h"


#define JD0 2451911 // дней до 01 янв 2001 ПН  по григорианскому календарю


void RTC_Configuration(void);
void RTC_config(void);
uint32_t FtimeToCounter();
void CounterToFtime(uint32_t counter);
void NVIC_config(void);


typedef struct{
uint16_t year;
uint8_t month;
uint16_t dweek;	
uint8_t day;
uint8_t hour;
uint8_t minute;
uint8_t second;
} ftime_t;

extern ftime_t ftime;


#endif 

