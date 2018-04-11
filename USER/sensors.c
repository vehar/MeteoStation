#include "sensors.h"

float DS18b20_temp = 0;
float TCoupleData = 0;	
int dh_T = 0;
int dh_H = 0;
uint16_t co2 = 0;
float internalTemp = 0;
uint16_t dustLvl = 0;

uint8_t ROM_SN[One_Wire_Device_Number_MAX][DS1822_SERIAL_NUM_SIZE];
uint8_t devices_cnt = 0;
float DS_Arr[128];

uint16_t getCO2Level()
{
 ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_55Cycles5); //External
 ADC_SoftwareStartConvCmd(ADC1, ENABLE);
 while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
 return ADC_GetConversionValue(ADC1);
}

float getInternalTemp()
{
 uint16_t result = 0;
 ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_71Cycles5); //Internal temperature
 ADC_SoftwareStartConvCmd(ADC1, ENABLE);
 while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
 result = ADC_GetConversionValue(ADC1);
 float v = result*0.000732421875; //*0.000732421875;
 float temp = (1.23-v)/0.005+25.0;
 return temp;
}

uint16_t Spi_Write_Data(uint16_t data)
{
	while(!(SPI2->SR & SPI_SR_TXE));//ждём пока опустошится Tx буфер 
	PIN_OFF(PB_1);//активируем Chip Select
	SPI2->DR = data;   //отправляем данные  
	while(!(SPI2->SR & SPI_SR_RXNE));//ждём пока придёт ответ
	data = SPI2->DR;  //считываем полученные данные 	 
	PIN_ON(PB_1);//деактивируем Chip Select 	
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


DECLARE_TASK(Ds18b20_Search)
{
	unsigned char cnt;
	__disable_irq();
	
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
	//	for (cnt2=0; cnt2!=8; cnt2++) uart_print_hex_value (USART1, ROM_SN[cnt][cnt2]);
		printf("\r\n");
	}
#endif	
	
	__enable_irq ();
}	

unsigned int temp[One_Wire_Device_Number_MAX];

DECLARE_TASK(Ds18b20_ReguestTemp)
{
	__disable_irq();

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

#endif	
	for (uint8_t cnt=0; cnt!=devices_cnt; cnt++)
	{
		One_Wire_Error_Handle(DS1822_Get_Conversion_Result_by_ROM_CRC(One_Wire_Pin, &ROM_SN[cnt], &temp[cnt]));
		DS18b20_temp =  (temp[cnt]>>4) + (float)((temp[cnt]&0x0F))/10;
		DS_Arr[cnt] = DS18b20_temp;
		//printf("DS:%d V:%d.%d \r\n", (cnt+1), (temp[cnt]>>4), (temp[cnt]&0x0F));
	}
	
	__enable_irq ();
}	


DECLARE_TASK(TermoCoupe_Hndl)
{
	TCoupleData = (Spi_Write_Data(0)>>3)*0.25;
	//printf("T1:%f \r\n", TCoupleData); 
}	


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
 
DECLARE_TASK(GasSensor_Hndl)
{
	co2 = getCO2Level();
	
	internalTemp = getInternalTemp();
	//printf("co2:%d \r\n", co2); 
}	


DECLARE_TASK(VibroSensor_Hndl)
{
		//Work vs vibrosensor 
	if(!PIN_SIGNAL(VIBRO_SENSOR_PIN))
		{
				
		}
}	

DECLARE_TASK(DustSensor_Hndl)
{
	__disable_irq ();
	PIN_OFF(DUST_PIN_LED_GND);
	PIN_OFF(DUST_PIN_LED_PWM);
	delay_ms(2);
	
	dustLvl = getCO2Level(); // B0
	
	delay_us(40);
	PIN_ON(DUST_PIN_LED_GND);
	PIN_ON(DUST_PIN_LED_PWM);
	
	//dustLvl = dustLvl*(3.3 / 1024) * 0.17 - 0.1;
	internalTemp = getInternalTemp();
	
	__enable_irq ();
}

