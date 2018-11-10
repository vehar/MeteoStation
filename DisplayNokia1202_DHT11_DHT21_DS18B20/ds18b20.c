#include "ds18b20.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

int ds18b20_temperature;
USART_InitTypeDef ds18b20_USART_InitStructure;

void ds18b20Init(void)
{ 
      GPIO_InitTypeDef  GPIO_InitStructure;   

      /* USART2 Tx (PA2) */
      GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;
      GPIO_Init(GPIOA, &GPIO_InitStructure);

      /* USART2 Rx (PA3)  */
      GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
      GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
      GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		   /*USART2 */
      ds18b20_USART_InitStructure.USART_BaudRate            = 115200;
      ds18b20_USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
      ds18b20_USART_InitStructure.USART_StopBits            = USART_StopBits_1;
      ds18b20_USART_InitStructure.USART_Parity              = USART_Parity_No ;
      ds18b20_USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
      ds18b20_USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
        
      /* USART2 */
      USART_Init(USART2, &ds18b20_USART_InitStructure);
      USART_Cmd(USART2, ENABLE);	
}

uint8_t ds18b20UsartReset(void) {
        uint8_t ow_presence;

        //USART_DMACmd(UART4, USART_DMAReq_Tx | USART_DMAReq_Rx, DISABLE);

        ds18b20_USART_InitStructure.USART_BaudRate = 9600;
        USART_Init(USART2, &ds18b20_USART_InitStructure);

        USART_ClearFlag(USART2, USART_FLAG_TC);

        USART_SendData(USART2, 0xf0);
         while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        ow_presence = USART_ReceiveData(USART2);

        ds18b20_USART_InitStructure.USART_BaudRate = 115200;
        USART_Init(USART2, &ds18b20_USART_InitStructure);

        if (ow_presence != 0xf0) {
                return 1;
        }

        return 0;
}


void OneWireSendByte(uint16_t byte)
{
	int i;
  for(i=0; i<8; i++)
  {
		USART_ClearFlag(USART2, USART_FLAG_TC);
    if(byte & (1<<i)) USART_SendData(USART2, 0xFF); else USART_SendData(USART2, 0x00);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
  }
}

int OneWireReadByte()
{
  unsigned short result = 0;
	int i;	
  
  for(i = 0; i < 16; i++)
  {
		USART_ClearFlag(USART2, USART_FLAG_TC);
    USART_SendData(USART2, 0xFF);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
		if(USART_ReceiveData(USART2) == 0xFF) result |= 1 << i;
  }
	
	if(result & 0x8000) 
		return (result >> 4) + 0xFFFFF000; 
	else 
		return result >>= 4;
}

int ds18b20GetTemperature(void) {
	return ds18b20_temperature;
}

void ds18b20Process(void) //1 Sec
{
	static int ds18b20_step = 0;
	
	switch(ds18b20_step) {
		case 0:
			ds18b20UsartReset();
			OneWireSendByte(0xCC);
			OneWireSendByte(0x44);	
			ds18b20_step++;
		break;
		
		case 1: 
			ds18b20UsartReset();
			OneWireSendByte(0xCC);
			OneWireSendByte(0xBE);
			ds18b20_temperature = OneWireReadByte();
			ds18b20_step = 0;
		break;
	}
}
