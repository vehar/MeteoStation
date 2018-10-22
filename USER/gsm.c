#include "gsm.h"
#include <stdio.h>
#include <string.h>

uint8_t GSM_state_f;
RingBuffer Gsm_RxBuff;
RingBuffer Gsm_TxBuff;

void GSM_MsgSend(uint8_t* data_buff, uint8_t sz)
{
	for(int i = 0; i<sz; i++)
	{
		WriteByte(&GSM_TX_BUFF, data_buff[i]);
	}
	
	USART_SendData(GSM_UART, ReadByte(&GSM_TX_BUFF));
	USART_ITConfig(GSM_UART, USART_IT_TC, ENABLE);
}

int GSM_MsgGet(uint8_t* data_buff)
{
	int sz = GetAmount(&GSM_RX_BUFF);
	uint8_t byte = 0;
	uint8_t k = 0;
	for(int i = 0; i<sz; i++)
	{
		byte = ReadByte(&GSM_RX_BUFF);
		if(byte != 0) //Skip zero bytes inside string!
		{
			data_buff[k++] = byte;
		}
	}
	return k;
}

void GSM_IRQHandler(void)
{
	char t = 0;
  if(USART_GetITStatus(GSM_UART, USART_IT_RXNE) != RESET)
  {
			t = USART_ReceiveData(GSM_UART);
			WriteByte(&GSM_RX_BUFF, t);
  }
	if(USART_GetITStatus(GSM_UART, USART_IT_TC) != RESET)
  {
    USART_ClearITPendingBit(GSM_UART, USART_IT_TC);
    if(!IsEmpty(&GSM_TX_BUFF))
		{
			USART_SendData(GSM_UART,ReadByte(&GSM_TX_BUFF));
		}
		else
		{
			USART_ITConfig(GSM_UART, USART_IT_TC, DISABLE);
		}
  }
}