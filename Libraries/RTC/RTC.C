/**************************************************************
** 	»рЕЈїЄ·ў°е
**	№¦ДЬЈєLEDКµСй
**  °ж±ѕЈєV1.0  
**	ВЫМіЈєwww.openmcu.com
**	МФ±¦Јєhttp://shop36995246.taobao.com/   
**  јјКхЦ§іЦИєЈє121939788 
***************************************************************/  
#include "stm32f10x.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include "rtc.h"

ftime_t ftime;

/*****************************************************************************
** єЇКэГыіЖ: TIM2_IRQHandler
** №¦ДЬГиКц: ¶ЁК±Жч2ЦР¶Пґ¦АнєЇКэ
				1usТ»ёцјЖКэ
** Чч  ЎЎХЯ: Dream
** ИХЎЎ  ЖЪ: 2010Дк12ФВ17ИХ
*****************************************************************************/ 
void RTC_Configuration(void)
{
  	/* Enable PWR and BKP clocks */
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  	/* Allow access to BKP Domain */
  	PWR_BackupAccessCmd(ENABLE);

  	/* Reset Backup Domain */
  	BKP_DeInit();

  	/* Enable LSE */
  	RCC_LSEConfig(RCC_LSE_ON);
  	/* Wait till LSE is ready */
  	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  	{}

  	/* Select LSE as RTC Clock Source */
  	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  	/* Enable RTC Clock */
  	RCC_RTCCLKCmd(ENABLE);

  	/* Wait for RTC registers synchronization */
  	RTC_WaitForSynchro();

  	/* Wait until last write operation on RTC registers has finished */
  	RTC_WaitForLastTask();

  	/* Enable the RTC Second */
  	RTC_ITConfig(RTC_IT_SEC, ENABLE);

  	/* Wait until last write operation on RTC registers has finished */
  	RTC_WaitForLastTask();

  	/* Set RTC prescaler: set RTC period to 1sec */
  	RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

  	/* Wait until last write operation on RTC registers has finished */
  	RTC_WaitForLastTask();
		
}


/*****************************************************************************
** єЇКэГыіЖ: RTC_IRQHandler
** №¦ДЬГиКц: RTCЦР¶ПєЇКэИлїЪ
					°ьАЁДкФВИХК±јд
** Чч  ЎЎХЯ: Dream
** ИХЎЎ  ЖЪ: 2010Дк12ФВ17ИХ
*****************************************************************************/
void RTC_IRQHandler(void)
{
  	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  	{
    	/* Clear the RTC Second interrupt */
    	RTC_ClearITPendingBit(RTC_IT_SEC);

	   	/* Wait until last write operation on RTC registers has finished */
    	RTC_WaitForLastTask();
    	// Reset RTC Counter when Time is 23:59:59 
    /*	if (RTC_GetCounter() >= 86399)  //0x00015180)
    	{
				
				RTC_SetCounter(0x00);
      	// Wait until last write operation on RTC registers has finished 
      	RTC_WaitForLastTask();
    	}
		*/
  	}
}

void RTC_config(void)
{
	
		if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  	{
    	/* Backup data register value is not correct or not yet programmed (when
       	the first time the program is executed) */

   // 	USART2_print_string("\r\n\n RTC not yet configured....",1);

    	/* RTC Configuration */
    	RTC_Configuration();

  //  	USART2_print_string("\r\n RTC configured....",1);

    	// Установка значения таймера по умолчанию 00:00:00
			
			RTC_WaitForLastTask();
    	/* Change the current time */
			RTC_SetCounter(0x00000);
			/* Wait until last write operation on RTC registers has finished */
			RTC_WaitForLastTask();
			

    	BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
  	}
  	else
  	{
    	
		/* Enable PWR and BKP clocks */
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  	/* Allow access to BKP Domain */
  	PWR_BackupAccessCmd(ENABLE);
			
			/* Check if the Power On Reset flag is set */
    	if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    	{
      //		USART2_print_string("\r\n\n Power On Reset occurred....",1);
    	}
    	/* Check if the Pin Reset flag is set */
    	else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    	{
      //		USART2_print_string("\r\n\n External Reset occurred....",1);
    	}

   // 	USART2_print_string("\r\n No need to configure RTC....",1);
    	/* Wait for RTC registers synchronization */
    	RTC_WaitForSynchro();

    	/* Enable the RTC Second */
    	RTC_ITConfig(RTC_IT_SEC, ENABLE);
    	/* Wait until last write operation on RTC registers has finished */
    	RTC_WaitForLastTask();
  	}

	#ifdef RTCClockOutput_Enable
  	/* Enable PWR and BKP clocks */
  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  	/* Allow access to BKP Domain */
  	PWR_BackupAccessCmd(ENABLE);

  	/* Disable the Tamper Pin */
  	BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
                                 functionality must be disabled */

  	/* Enable RTC Clock Output on Tamper Pin */
  	BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
	#endif

  	/* Clear reset flags */
  	RCC_ClearFlag();
	
		NVIC_config();
}

// функция преобразования григорианской даты и времени в значение счетчика
// функия конвертации для григореаского времени взяты отсюда:  http://we.easyelectronics.ru/Soft/funkcii-kalendarya-i-vremeni-na-odnom-registre.html
// текущий юлианский день(JD0) можно узнать тут: https://ru.wikipedia.org/wiki/?????????_???? 
uint32_t FtimeToCounter()
{
uint8_t a;
uint16_t y;
uint8_t m;
uint32_t JDN;

// вычисление необходимых коэффициентов
a=(14-ftime.month)/12;
y=ftime.year+4800-a;
m=ftime.month+(12*a)-3;
// вычисляем значение текущего Юлианского дня
JDN=ftime.day;
JDN+=(153*m+2)/5;
JDN+=365*y;
JDN+=y/4;
JDN+=-y/100;
JDN+=y/400;
JDN+=-32045;
JDN+=-JD0; // т.к. счетчик у нас не резиновый, убираем дни, которые прошли до 01 янв 2001 
JDN*=86400;     // переводим дни в секунды
JDN+=(ftime.hour*3600); // и дополняем их секундами текущего дня
JDN+=(ftime.minute*60);
JDN+=(ftime.second);
// итого имеем колличество секунд с 00-00 01 янв 2001
return JDN;
}


//  функция преобразования счетчика в григореанскую дату и время
void CounterToFtime(uint32_t counter)
{
uint32_t ace;
uint8_t b;
uint8_t d;
uint8_t m;

uint8_t  Am;
uint16_t Ym;
uint8_t  Mm;
	
ace=(counter/86400)+32044+JD0;
b=(4*ace+3)/146097; // может ли произойти потеря точности из-за переполнения 4*ace ??
ace=ace-((146097*b)/4);
d=(4*ace+3)/1461;
ace=ace-((1461*d)/4);
m=(5*ace+2)/153;
ftime.day=ace-((153*m+2)/5)+1;
ftime.month=m+3-(12*(m/10));
ftime.year=100*b+d-4800+(m/10);
ftime.hour=(counter/3600)%24;
ftime.minute=(counter/60)%60;
ftime.second=(counter%60);
/*
Алгоритм вычисления текущего дня недели по известным год, месяц, дата
a = (14 - месяц) / 12
y = год - a
m = месяц + 12 * a - 2
День недели = (7000 + (тек. День + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12)) ост от деления(MOD) 7
0 - Воскресенье, 1- Понедельник и т.д

*/	

Am = (14-ftime.month)/12;
Ym = ftime.year - Am;
Mm = ftime.month + 12*Am - 2;
ftime.dweek = (7000 + (ftime.day + Ym + Ym/4 - Ym/100 + Ym/400 + (31*Mm)/12));
ftime.dweek = ftime.dweek % 7;

}


void NVIC_config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;


  	/* Enable the RTC Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
	
}

/*******************************************************************************
** End of File
********************************************************************************/
