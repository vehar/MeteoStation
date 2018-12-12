#include "main.h"
#include "init.h"
#include "RingBuffer.h"
#include "sensors.h"
#include "radiolink.h"
#include "mesh.h"
#include "gsm.h"
#include "EERTOS.h"
#include "rtc.h"

#include "Sim80xConfig.h"
#include "Sim80x.h"
#include "pms.h"

#include "debug.h"

#define VERBOSE_OUTPUT

uint8_t verboseOutput = 0;
 
FILE __stdout;
FILE __stdin;
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */  
 
DECLARE_TASK(Radio_to_GSM);
DECLARE_TASK(GSM_to_Radio);
DECLARE_TASK(T_HeartBit);
	
int fputc(int ch, FILE *f) 
{
  /*if (DEMCR & TRCENA) // for SWO out
	{
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }*/
	uart_send_char(USART1, ch);//Radio uart
	//uart_send_char(USART3, ch); //GSM uart
  return(ch);
}  

PMS_DATA* data;
uint8_t pms_msg[20];

DECLARE_TASK(PmsRead)
{
  if (PMS_read(data, pms_msg))
  {
    printf("PM 1.0 (ug/m3): %i\r\n", data->PM_AE_UG_1_0);
    printf("PM 2.5 (ug/m3): %i\r\n", data->PM_AE_UG_2_5);
    printf("PM 10.0 (ug/m3): %i\r\n", data->PM_AE_UG_10_0);
  }
}

//TODO: add stm temperature + calibration
//TODO: add vibro and butt IRQ
//TODO: fix DHT_CS_ERROR for DHT

// http://we.easyelectronics.ru/STM32/stm32-usart-na-preryvaniyah-na-primere-rs485.html 
int main(void)
{
	MeshInit();
	
	ClearBuf(&Radio_RxBuff); 
	Delay_Init();
	GPIO_Configuration();
	USART_Configuration();
	Adc_Init();
	RTC_config();
	
	DEBUGFMSG("Init done");
	
	RTOS_timer_init();
  RunRTOS();
  __enable_irq ();

	/*uint8_t CSQ[] = {"CSQ"};
	AT_GSM_MsgSendV(CSQ,"?");
	AT_GSM_MsgSendV(CSQ,"2,1");
	AT_GSM_MsgSendV(CSQ,"3,1,","CONTYPE",",","GPRS");*/

	
//	 printf("CONFIG = %s\r\n", IsMaster ? "Host" : "Slave");

	//SetTimerTaskInfin(RadioRead_T, 0, 100);
	//SetTimerTaskInfin(RadioBroadcast_T, 0, 1000);	
	
/*	
	ftime.day = 1;
	ftime.month = 2;
	ftime.year = 2018;
	ftime.hour = 3;
	ftime.minute = 4;
	ftime.second = 0;
	
	// Change the current time 
	RTC_SetCounter(FtimeToCounter());
	// Wait until last write operation on RTC registers has finished 
	RTC_WaitForLastTask();
*/	
												
	SetTimerTaskInfin(T_HeartBit, 0, 1000);
	SetTimerTaskInfin(GetInternalsParams, 0, 1000);
	
	if(IsMaster)
	{	
//		printf("HOST MAC:%x:%x:%x:%x \r\n", HostID.off0, HostID.off2, HostID.off4, HostID.off8); 	
		
		SetTimerTaskInfin(Ds18b20_Search, 0, 15000);
		SetTimerTaskInfin(Ds18b20_ReguestTemp, 0, 1000);
	//	SetTimerTaskInfin(Humidity_Hndl, 0, 1000);
		SetTimerTaskInfin(GasSensor_Hndl, 0, 100);
		
		SetTimerTaskInfin(InfoOut_T, 0, 1500);
	}
	else
	{
		//PIN_CONFIGURATION(DUST_PIN_ANALOG);
		
		//SetTimerTaskInfin(Humidity_Hndl, 0, 3000);
	//	HC12_configBaud(1152);
	//	SetTimerTaskInfin(GSM_FTP_Connect, 0, 100);
		
//		SetTimerTaskInfin(PmsRead, 10, 1000);
	//SetTimerTaskInfin(GSM_Lib, 10, 0);
		
//		SetTimerTaskInfin(GSM_Actions, 0, 1000);

		SetTimerTaskInfin(GSM_to_Radio, 0, 100);
		SetTimerTaskInfin(Radio_to_GSM, 0, 150);
		//SetTimerTaskInfin(DustSensor_Hndl, 0, 100);
	}

	
	DEBUGFMSG("Start Sheduller..\r\n");	 

	Shedull(1);//зациклились через диспетчер
	
	while(1){}// Fatal RTOS error
}

DECLARE_TASK(T_HeartBit)
{
	static uint8_t t = 0;
	if(IsMaster)
	{
	//	if(!t){PIN_ON(BOARD_LED);}
	//	else{PIN_OFF(BOARD_LED);}
	}
	else //MAPPLE_LED
	{
	//	if(!t){PIN_ON(MAPPLE_LED);}
	//	else{PIN_OFF(MAPPLE_LED);}
	}
	t = !t;
	
	if(TaskExist(StartSim80xTask) == 0) {DEBUGFMSG("\r\StartSim80xTask ---> ERROR\r\n");}
	if(TaskExist(StartSim80xBuffTask) == 0) {DEBUGFMSG("\r\StartSim80xBuffTask ---> ERROR\r\n");}
	
	sFile.rtcRaw = RTC_GetCounter();
	CounterToFtime(sFile.rtcRaw);
}	 

DECLARE_TASK(GSM_to_Radio)
{
	uint8_t GRmsgBuff[50];
	memset(GRmsgBuff, 0, 50);
	uint8_t size = GSM_MsgGet(GRmsgBuff);
	if(size != 0) {RadioB_MsgSend(GRmsgBuff, size);}
}

DECLARE_TASK(Radio_to_GSM)
{
	uint8_t RGmsgBuff[50];
	memset(RGmsgBuff, 0, 50);	
	uint8_t size = RadioB_MsgGet(RGmsgBuff);
	if(size != 0) {GSM_MsgSend(RGmsgBuff, size); /*printf(">> %s", msgBuff);*/}
}

void TIM2_IRQHandler(void)
{
	if ( TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET ) 
	{
	   TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		 RTOS_TIMER_ISR_HNDL();
  }	
}

void SysTick_Handler (void)
{
    
}
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

