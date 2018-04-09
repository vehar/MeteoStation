#include "main.h"
#include "init.h"
#include "RingBuffer.h"
#include "sensors.h"

#include "EERTOS.h"
//Termopare https://hubstub.ru/stm32/141-izmerenie-temperatury-s-pomoschyu-termopary-na-primere-max6675-dlya-stm32.html
//Gas http://catethysis.ru/stm32-mq135/
//Preasure http://ziblog.ru/2013/03/15/bmp085-datchik-davleniya.html


#define VERBOSE_OUTPUT



DECLARE_TASK(T_HeartBit);


FILE __stdout;
FILE __stdin;

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */  
  
CircularBuffer RxBuff;
  
int fputc(int ch, FILE *f) 
{
  /*if (DEMCR & TRCENA) // for SWO out
	{
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }*/
	uart_send_char(USART1, ch);//Debug uart
	uart_send_char(USART3, ch); //Radio uart
  return(ch);
}

DECLARE_TASK(InfoOut_T)
{
	
#ifdef VERBOSE_OUTPUT	
	
printf(" \r\n\r\n"); 
	
	printf("---------Temperature---------\r\n"); 
	for (uint8_t cnt=0; cnt!=devices_cnt; cnt++)
	{
		DS18b20_temp = DS_Arr[cnt];
		printf("DS_n: %d - T:%2.2f \r\n", cnt, DS18b20_temp);
	}
	 
	printf("TC T:%2.2f \r\n", TCoupleData); 
	
	if(dh_T == 0xFFFFFC00){ printf("DHT11 T: ---\r\n"); }
		else { printf("DHT11 T: %i\r\n", dh_T); }
	
//	printf("MCU T:%2.2f \r\n", 0); 
		
	printf("---------Humidity---------\r\n"); 
	
	if(dh_H == 0xFFFFFC00){ printf("DHT11 H: ---\r\n"); }
		else { printf("DHT11 H: %i\r\n", dh_H); }
		
	printf("---------Gas_level---------\r\n"); 	
	printf("CO2:%d \r\n", co2); 
	
printf(" \r\n\r\n"); 		

#else
		printf("$%d, %d, %d, %d, %d, %d;", (int)(DS_Arr[0]*100), (int)(TCoupleData*100), (int)(dh_T*100),(int)dh_H, (int)(co2), 1);
		//printf("$%d, %d, %d;", DS18b20_temp, dh_H, co2);
#endif // VERBOSE_OUTPUT	
}	


DECLARE_TASK(RadioRead_T)
{
	char t;
	do
	{
		t = ReadByte(&RxBuff);
		printf("%c", t);
	}while(t != 0xFF);
}


//TODO: add stm temperature + calibration
//TODO: add vibro and butt IRQ
//TODO: fix DHT_CS_ERROR for DHT
int main(void)
{
	ClearBuf(&RxBuff); 
	Delay_Init();
	
	GPIO_Configuration();
	USART_Configuration();
	
	Spi_Init();
	Adc_Init();
	
	RTOS_timer_init();
	memset(DS_Arr, 128, 0xFF);
	
  RunRTOS();
  __enable_irq ();

	
	SetTimerTaskInfin(T_HeartBit, 0, 1000);
	SetTimerTaskInfin(Ds18b20_Search, 0, 15000);
	SetTimerTaskInfin(Ds18b20_ReguestTemp, 0, 1000);
	SetTimerTaskInfin(TermoCoupe_Hndl, 0, 500);
	SetTimerTaskInfin(Humidity_Hndl, 0, 1000);
	SetTimerTaskInfin(GasSensor_Hndl, 0, 100);
	SetTimerTaskInfin(VibroSensor_Hndl, 0, 1000);
	
	SetTimerTaskInfin(InfoOut_T, 0, 1500);
	SetTimerTaskInfin(RadioRead_T, 0, 1500);
	
	
	printf("Start......\r\n");	 

	Shedull(1);//зациклились через диспетчер
	
	while(1){}// Fatal RTOS error
}


void SysTick_Handler (void)
{
    
}

void TIM2_IRQHandler(void)
{
	if ( TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET ) 
	{
	   TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		 RTOS_TIMER_ISR_HNDL();
  }	
}


void USART1_IRQHandler(void)
{
	char t = 0;
    if ((USART1->SR & USART_FLAG_RXNE) != (u16)RESET)
    {		
       t = USART_ReceiveData(USART1);
	   WriteByte(&RxBuff, t);
    }
}


void USART3_IRQHandler(void)
{
    if ((USART3->SR & USART_FLAG_RXNE) != (u16)RESET)
    {
      USART_ReceiveData(USART3);
    }
}
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

