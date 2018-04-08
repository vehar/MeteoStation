#include "main.h"
#include "init.h"

#include "EERTOS.h"
//Termopare https://hubstub.ru/stm32/141-izmerenie-temperatury-s-pomoschyu-termopary-na-primere-max6675-dlya-stm32.html
//Gas http://catethysis.ru/stm32-mq135/
//Preasure http://ziblog.ru/2013/03/15/bmp085-datchik-davleniya.html


//#define VERBOSE_OUTPUT


DECLARE_TASK(VibroSensor_Hndl);
DECLARE_TASK(GasSensor_Hndl);
DECLARE_TASK(Humidity_Hndl);
DECLARE_TASK(TermoCoupe_Hndl);
DECLARE_TASK(Ds18b20_Hndl);
DECLARE_TASK(Ds18b20_ReguestTemp);
DECLARE_TASK(Ds18b20_Search);
DECLARE_TASK(Blink);


FILE __stdout;
FILE __stdin;

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */  
  

int fputc(int ch, FILE *f) 
{
  /*if (DEMCR & TRCENA) // for SWO out
	{
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }*/
	uart_send_char(USART1, ch);
	
  return(ch);
}


uint8_t ROM_SN[One_Wire_Device_Number_MAX][DS1822_SERIAL_NUM_SIZE];
uint8_t devices_cnt;
float DS_Arr[128];

  
uint16_t getCO2Level()
{
 ADC_SoftwareStartConvCmd(ADC1, ENABLE);
 while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
 return ADC_GetConversionValue(ADC1);
}


uint16_t Spi_Write_Data(uint16_t data)
{
	while(!(SPI2->SR & SPI_SR_TXE));//ждём пока опустошится Tx буфер
	CS_LOW   //активируем Chip Select
	SPI2->DR = data;   //отправляем данные  
	while(!(SPI2->SR & SPI_SR_RXNE));//ждём пока придёт ответ
	data = SPI2->DR;  //считываем полученные данные 
	CS_HIGH 	//деактивируем Chip Select  
  return data;   //возвращаем то, что прочитали
}

unsigned char One_Wire_Error_Handle (unsigned char err)
{
#ifdef VERBOSE_OUTPUT
	switch (err)
	{
	 	case One_Wire_Success: 					  printf("Success!"); 					break;
	 	case One_Wire_Error_No_Echo: 			printf("No echo recieved!");  break;
	 	case One_Wire_Bus_Low_Error: 			printf("Pin LOW error!"); 		break;
	 	case One_Wire_CRC_Error: 				  printf("CRC not match!"); 		break;
	}
#endif	
	if (err==One_Wire_Success) {return 0; }
	else {return 1;} //while(0); //or some error handler
}

////////////////////////////////////////////////////////////////////////////


DECLARE_TASK(Blink)
{
	static uint8_t t = 0;
	if(!t)
	{
		PIN_ON(BOARD_LED);
	}
	else
	{
		PIN_OFF(BOARD_LED);
	}
	t = !t;
}	


DECLARE_TASK(Ds18b20_Search)
{
		unsigned char cnt;
	__disable_irq ();
	
#ifdef VERBOSE_OUTPUT	
	printf("Checking 1-Wire bus...\r\n");
#endif
	
	One_Wire_Error_Handle(One_Wire_Reset(One_Wire_Pin));
	One_Wire_Error_Handle(DS1822_Search_Rom2(One_Wire_Pin, &devices_cnt, &ROM_SN)); //Sending SearchROM command...

#ifdef VERBOSE_OUTPUT	
	printf("Found %d sensor(s) \r\n", devices_cnt);
		
	for (cnt=0; cnt!=devices_cnt; cnt++)
	{
		unsigned char cnt2;
		printf("Sensor %d  serial number: ", cnt);
		for (cnt2=0; cnt2!=8; cnt2++) uart_print_hex_value (USART1, ROM_SN[cnt][cnt2]);
		printf("\r\n");
	}
#endif	
	
	__enable_irq ();
}	

unsigned int temp[One_Wire_Device_Number_MAX];

DECLARE_TASK(Ds18b20_ReguestTemp)
{
		__disable_irq ();

#ifdef VERBOSE_OUTPUT		
	printf("Starting convertion proccess...\r\n");
#endif
	
	for (uint8_t cnt=0; cnt!=devices_cnt; cnt++)
	{
		One_Wire_Error_Handle(DS1822_Start_Conversion_by_ROM(One_Wire_Pin, &(ROM_SN[cnt])));
	}
	
#ifdef VERBOSE_OUTPUT		
	printf("Waiting 750ms for conversion ready\r\n");
	//for (uint8_t cnt=0;cnt!=75;cnt++)	{	printf(".");delay_ms(10);	}
#endif		
	__enable_irq ();
	
	SetTimerTaskInfin(Ds18b20_Hndl, 750, 0);
}

float DS18b20_temp = 0;
DECLARE_TASK(Ds18b20_Hndl)
{
		__disable_irq ();
#ifdef VERBOSE_OUTPUT		
	printf("\r\n Getting results...\r\n");
	
	for (uint8_t cnt=0; cnt!=devices_cnt; cnt++)
	{
		One_Wire_Error_Handle(DS1822_Get_Conversion_Result_by_ROM_CRC(One_Wire_Pin, &ROM_SN[cnt], &temp[cnt]));
		printf("Sensor %d Temperature value: %d.%d *C\r\n", (cnt+1), (temp[cnt]>>4), (temp[cnt]&0x0F));
	}
#else
		for (uint8_t cnt=0; cnt!=devices_cnt; cnt++)
	{
		One_Wire_Error_Handle(DS1822_Get_Conversion_Result_by_ROM_CRC(One_Wire_Pin, &ROM_SN[cnt], &temp[cnt]));
		DS18b20_temp =  (temp[cnt]>>4) + (float)((temp[cnt]&0x0F))/10;
		DS_Arr[cnt] = DS18b20_temp;
		//printf("DS:%d V:%d.%d \r\n", (cnt+1), (temp[cnt]>>4), (temp[cnt]&0x0F));
	}
#endif	
	__enable_irq ();
}	


float TCoupleData;	

DECLARE_TASK(TermoCoupe_Hndl)
{
	TCoupleData = (Spi_Write_Data(0)>>3)*0.25;
	//printf("T1:%f \r\n", TCoupleData); 
}	

int dh_T = 0;
int dh_H = 0;
//Humidity sensor ВРЕ11 http://ffix.ru/index.php/stm32-stm8/21-http-ffixru-component-k2-itemlist-user-63-dmitrijhtml 
DECLARE_TASK(Humidity_Hndl)
{
		//__disable_irq ();
	dht_process(DHT11);
	
	dh_T = dhtGet(DHT_TEMP, DHT11); 
	dh_H = dhtGet(DHT_HUM,  DHT11);
	/*
	if(dh_T == 0xFFFFFC00){ printf("DHT11 T: ---\r\n"); }
		else { printf("DHT11 T: %i\r\n", dh_T); }
	if(dh_H == 0xFFFFFC00){ printf("DHT11 H: ---\r\n"); }
		else { printf("DHT11 H: %i\r\n", dh_H); }*/
		//__enable_irq ();
}

uint16_t co2;
 
DECLARE_TASK(GasSensor_Hndl)
{
	co2 = getCO2Level();
	//printf("co2:%d \r\n", co2); 
}	


DECLARE_TASK(VibroSensor_Hndl)
{
		//Work vs vibrosensor 
	if(!PIN_SIGNAL(VIBRO_SENSOR_PIN))
		{
				
		}
}	

DECLARE_TASK(InfoOut_T)
{
	/*
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
*/
		printf("$%d, %d, %d, %d, %d, %d;", (int)(DS_Arr[0]*100), (int)(TCoupleData*100), (int)(dh_T*100),(int)dh_H, (int)(co2), 1);
		//printf("$%d, %d, %d;", DS18b20_temp, dh_H, co2);
}	

//http://www.avislab.com/blog/stm32-rtc/
unsigned char RTC_Init(void)
{
    // Дозволити тактування модулів управління живленням і управлінням резервної областю
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Дозволити доступ до області резервних даних
    PWR_BackupAccessCmd(ENABLE);
    // Якщо годинник вимкнений - ініціалізувати
    if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
    {
        // Виконати скидання області резервних даних
        RCC_BackupResetCmd(ENABLE);
        RCC_BackupResetCmd(DISABLE);
 
        // Вибрати джерелом тактових імпульсів зовнішній кварц 32768 і подати тактування
        RCC_LSEConfig(RCC_LSE_ON);
        while ((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY) {}
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
 
        RTC_SetPrescaler(0x7FFF); // Встановити поділювач, щоб годинник рахував секунди
 
        // Вмикаємо годинник
        RCC_RTCCLKCmd(ENABLE);
 
        // Чекаємо на синхронізацію
        RTC_WaitForSynchro();
 
				//NVIC_EnableIRQ (RTC_IRQn);           //разрешить прерывания от RTC
        return 1;
    }
    return 0;
}

//TODO: add stm temperature + calibration
//TODO: add vibro and butt IRQ
//TODO: fix DHT_CS_ERROR for DHT
int main(void)
{
	Delay_Init();
	
	GPIO_Configuration();
	USART_Configuration();
	Spi_Init();
	Adc_Init();
	
	RTOS_timer_init();
	memset(DS_Arr, 128, 0xFF);
	
  RunRTOS();
  __enable_irq ();

	
	SetTimerTaskInfin(Blink, 0, 1000);
	SetTimerTaskInfin(Ds18b20_Search, 0, 15000);
	SetTimerTaskInfin(Ds18b20_ReguestTemp, 0, 1000);
	SetTimerTaskInfin(TermoCoupe_Hndl, 0, 500);
	SetTimerTaskInfin(Humidity_Hndl, 0, 1000);
	SetTimerTaskInfin(GasSensor_Hndl, 0, 100);
	SetTimerTaskInfin(VibroSensor_Hndl, 0, 1000);
	
	SetTimerTaskInfin(InfoOut_T, 0, 500);
	
	
	printf("Start......\r\n");	 

	Shedull(1);//зациклились через диспетчер
	while(1)
	{
		// Fatal RTOS error
	}
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

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
