#include "main.h"
#include "init.h"
#include "RingBuffer.h"
#include "sensors.h"
#include "radiolink.h"
#include "EERTOS.h"

#define VERBOSE_OUTPUT

uint8_t HostIDArr[] = {0x48, 0xFF, 0x6C, 0x06, 0x50, 0x77, 0x51, 0x49, 0x43, 0x35, 0x20, 0x87};//HOST ID
uint8_t Slave1IDArr[] = {0x52, 0xFF, 0x70, 0x06, 0x49, 0x84, 0x55, 0x50, 0x12, 0x36, 0x11, 0x87};// S1 ID

uint8_t verboseOutput = 0;

bool IsMaster;

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
	uart_send_char(USART1, ch);//Debug uart
//	uart_send_char(USART3, ch); //Radio uart
  return(ch);
}  

uint8_t Rxdh_T = 0;
uint8_t Rxdh_H = 0;
uint16_t RxdustLvl = 0;

uint8_t data_buff[27];
bool RemoteConnected = 0;
bool MasterSync = 0;

DECLARE_TASK(T_HeartBit)
{
	static uint8_t t = 0;
	if(IsMaster)
	{
		if(!t){PIN_ON(BOARD_LED);}
		else{PIN_OFF(BOARD_LED);}
	}
	else //MAPPLE_LED
	{
		if(!t){PIN_ON(MAPPLE_LED);}
		else{PIN_OFF(MAPPLE_LED);}
	}
	t = !t;
}	 

DECLARE_TASK(InfoOut_T)
{	
	if(verboseOutput)
	{	
		printf(" \r\n\r\n"); 
	
		printf("---------Temperature---------\r\n"); 
		for (uint8_t cnt=0; cnt!=devices_cnt; cnt++)
		{
			DS18b20_temp = DS_Arr[cnt];
			printf("DS_n: %d - T:%2.2f \r\n", cnt, DS18b20_temp);
		}
		 
		printf("TC T:%2.2f \r\n", TCoupleData); 
		
		//if(dh_T == 0xFFFFFC00){ printf("DHT11 T: ---\r\n"); }
		//else { printf("DHT11 T: %i\r\n", dh_T); }
		
		printf("Internal:%f \r\n", internalTemp); 
			
		//printf("---------Humidity---------\r\n"); 
		
		//if(dh_H == 0xFFFFFC00){ printf("DHT11 H: ---\r\n"); }
		//else { printf("DHT11 H: %i\r\n", dh_H); }
			
		printf("---------Gas_level---------\r\n"); 	
		printf("CO2:%d \r\n", co2); 
		
		if(RemoteConnected)
		{
			printf("---------Remote sensor data---------\r\n"); 
			printf(" dustLvl:%d \r\n", RxdustLvl); 				
		
			if(Rxdh_T == 0xFFFFFC00){ printf("\t DHT11 T: ---\r\n"); }
			else { printf(" DHT11 T: %i\r\n", Rxdh_T); }
			if(Rxdh_H == 0xFFFFFC00){ printf("\t DHT11 H: ---\r\n"); }
			else { printf(" DHT11 H: %i\r\n", Rxdh_H); }
		}
	
	printf(" \r\n\r\n"); 			
	}
	else
	{
			printf("$%d, %d, %d, %d, %d, %d;", (int)(DS_Arr[0]*100), (int)(TCoupleData*100), (int)(Rxdh_T*100),(int)Rxdh_H, (int)(co2), 1);
	}// VERBOSE_OUTPUT	
}	



DECLARE_TASK(RadioRead_T)
{
	float recievedInternalTemp = 0;
	struct u_id slaveID;
	struct u_id RXslaveID;
	
	memcpy(&slaveID, &Slave1IDArr, sizeof(Slave1IDArr));
	
	/*char t;
	t = ReadByte(&RxBuff);
	if(t != 0xFF){printf("%c", t);}*/
	RadioPack_t pack;
	memset(&pack, 0, sizeof(pack));
	
	uint16_t i = 0;
	__disable_irq();
	
	//printf("GetAmount:%d \r\n", GetAmount(&RxBuff)); 
	
	if(GetAmount(&RxBuff) >= 25) //full pack recieved!
	{		
		for(int i = 0; i<=26; i++)
		{
			data_buff[i] = ReadByte(&RxBuff);
		}		
		memcpy(&pack, data_buff, 24);
		RXslaveID = pack.senderId;
		
		if(pack.msgId == 0xBB) //From slave
		{
			if(uid_cmp(&RXslaveID, &slaveID))
			{
				if(!RemoteConnected)
				{
					printf("REMOTE \r\n"); 
					printf("\t MAC:%x:%x:%x:%x \r\n", pack.senderId.off0, pack.senderId.off2, pack.senderId.off4, pack.senderId.off8); 
					printf("\t ID:%x \r\n", pack.msgId); 
					RemoteConnected = 1;
				}
				
				RxdustLvl = pack.data[0];
				Rxdh_T = pack.data[1];
				Rxdh_H = pack.data[2];			
			}
		}
		else if((pack.msgId == 0xAA) && (MasterSync == 0)) //sunc vs master
		{
			SetTimerTaskInfin(T_HeartBit, 0, 100);
			ClearTimerTask(RadioRead_T); //Stop listening
		}
		//MsgGet(&pack, data_buff);
		ClearBuf(&RxBuff);
	}
	__enable_irq();
}
DECLARE_TASK(RadioBroadcast_T)
{
	uint8_t data_buff[8];
	for(int i = 0; i<8; i++){data_buff[i]=0;}	
	if(IsMaster)
	{
		MsgSend(0xAA, data_buff);
	}
	else
	{
		data_buff[0] = dustLvl;
		//data_buff[1] = (uint8_t)internalTemp;
		//data_buff[2] = (uint8_t)(internalTemp*100)>>2;
		data_buff[1] = dh_T;
		data_buff[2] = dh_H;
		
		MsgSend(0xBB, data_buff);
	}
}


/*	
$48
$FF
$6C
$06
$50
$77
$51
$49
$43
$35
$20
$87
$00
$01
$02
$03
$04
$05
$06
$07
$AA
$00
$00
$00
$99
*/

//TODO: add stm temperature + calibration
//TODO: add vibro and butt IRQ
//TODO: fix DHT_CS_ERROR for DHT

// http://we.easyelectronics.ru/STM32/stm32-usart-na-preryvaniyah-na-primere-rs485.html 
int main(void)
{
	uid_read(&HostID);
	struct u_id idn;
	memcpy(&idn, &HostIDArr, sizeof(HostIDArr));
    IsMaster = uid_cmp(&HostID, &idn);
   
	
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

	
	 printf("CONFIG = %s\r\n", IsMaster ? "Host" : "Slave");

	SetTimerTaskInfin(RadioRead_T, 0, 100);
	SetTimerTaskInfin(RadioBroadcast_T, 0, 1000);	
	SetTimerTaskInfin(T_HeartBit, 0, 1000);
	
	if(IsMaster)
	{	
		printf("HOST MAC:%x:%x:%x:%x \r\n", HostID.off0, HostID.off2, HostID.off4, HostID.off8); 	
		
		SetTimerTaskInfin(Ds18b20_Search, 0, 15000);
		SetTimerTaskInfin(Ds18b20_ReguestTemp, 0, 1000);
		SetTimerTaskInfin(TermoCoupe_Hndl, 0, 500);
	//	SetTimerTaskInfin(Humidity_Hndl, 0, 1000);
		SetTimerTaskInfin(GasSensor_Hndl, 0, 100);
		SetTimerTaskInfin(VibroSensor_Hndl, 0, 1000);
		
		SetTimerTaskInfin(InfoOut_T, 0, 1500);
	}
	else
	{
		PIN_CONFIGURATION(MAPPLE_LED);	
		PIN_CONFIGURATION(DUST_PIN_LED_GND);
		PIN_CONFIGURATION(DUST_PIN_LED_PWM);
		//PIN_CONFIGURATION(DUST_PIN_ANALOG);
		
		SetTimerTaskInfin(Humidity_Hndl, 0, 1000);
		SetTimerTaskInfin(DustSensor_Hndl, 0, 100);
	}

	
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
	   //WriteByte(&RxBuff, t);
    }
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

