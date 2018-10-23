#include <stdio.h>
#include <string.h>
#include "radiolink.h"
#include "RingBuffer.h"
#include "mesh.h"

#include "debug.h"

RingBuffer Radio_RxBuff;
RingBuffer Radio_TxBuff;

//48 FF 6C 06 50 77 51 49 43 35 20 87 HOST ID
int sz = 0;
int Radio_MsgSend(uint32_t msgId, uint8_t* data_buff)
{
	struct u_id senderId;
	RadioPack_t pack;
	uint8_t elem = 0;
	uint8_t* elem_p;
	uint8_t crc = 0;
	
	sz = sizeof(pack);
	memset(&pack, 0, sizeof(pack));
	
	pack.senderId = HostID;
	pack.msgId = msgId;
	memcpy(pack.data, data_buff, 8);
	elem_p = (uint8_t*)&pack;
	for(int i = 0; i<sz; i++)
	{		
		elem = *(elem_p+i);
		crc ^= elem;
		WriteByte(&TX_BUFF, elem);
	}
	pack.crc = crc;
	WriteByte(&TX_BUFF, pack.crc);
	
	USART_SendData(RADIO_UART, ReadByte(&TX_BUFF));
	USART_ITConfig(RADIO_UART, USART_IT_TC, ENABLE);
}

int Radio_MsgGet(RadioPack_t* pack, uint8_t* data_buff)
{
	uint8_t crc = 0;
	memset(&pack, 0, sizeof(pack));
	crc = 0;
	uint8_t elem = 0;
	uint8_t* elem_p;
	
	elem_p = (uint8_t*)&pack;
	for(int i = 0; i<sz; i++)
	{		
		elem = ReadByte(&RX_BUFF);	
		crc ^= elem;
		*(elem_p+i) = elem;
	}
	pack->crc = ReadByte(&RX_BUFF);
	return (pack->crc == crc);	
}


void RadioB_MsgSend(uint8_t* data_buff, uint8_t sz)
{
	for(int i = 0; i<sz; i++)
	{
		WriteByte(&TX_BUFF, data_buff[i]);
	}
	
	USART_SendData(RADIO_UART, ReadByte(&TX_BUFF));
	USART_ITConfig(RADIO_UART, USART_IT_TC, ENABLE);
}

int RadioB_MsgGet(uint8_t* data_buff)
{
	int sz = GetAmount(&RX_BUFF);
	//if(sz>0){sz -= 1;}
	for(int i = 0; i<sz; i++)
	{
		data_buff[i] = ReadByte(&RX_BUFF);
	}
	return sz;
}

//TODO:
void HC12_configBaud(int baud)
{
	uint8_t buff[20];
	memset(buff, 0, 20);
	PIN_OFF(RADIO_SET);
	delay_ms(1000);
//	RadioB_MsgSend("AT\r\n");
	delay_ms(500);
	RadioB_MsgGet(buff);
	PIN_ON(RADIO_SET);
}

DECLARE_TASK(RadioRead_T)
{
	float recievedInternalTemp = 0;
	struct u_id slaveID;
	struct u_id RXslaveID;
	
	memcpy(&slaveID, &Slave1IDArr, sizeof(Slave1IDArr));
	
	/*char t;
	t = ReadByte(&Radio_RxBuff);
	if(t != 0xFF){printf("%c", t);}*/
	RadioPack_t pack;
	memset(&pack, 0, sizeof(pack));
	
	uint16_t i = 0;
	__disable_irq();
	
	//printf("GetAmount:%d \r\n", GetAmount(&Radio_RxBuff)); 
	
	if(GetAmount(&Radio_RxBuff) >= 25) //full pack recieved!
	{		
		for(int i = 0; i<=26; i++)
		{
			data_buff[i] = ReadByte(&Radio_RxBuff);
		}		
		memcpy(&pack, data_buff, 24);
		RXslaveID = pack.senderId;
		
		if(pack.msgId == 0xBB) //From slave
		{
			if(uid_cmp(&RXslaveID, &slaveID))
			{
				if(!RemoteConnected)
				{
					DEBUGMSG("REMOTE \r\n"); 
					DEBUGMSG("\t MAC:%x:%x:%x:%x \r\n", pack.senderId.off0, pack.senderId.off2, pack.senderId.off4, pack.senderId.off8); 
					DEBUGMSG("\t ID:%x \r\n", pack.msgId); 
					RemoteConnected = 1;
				}
				
//				RxdustLvl = pack.data[0];
//				Rxdh_T = pack.data[1];
//				Rxdh_H = pack.data[2];			
			}
		}
		else if((pack.msgId == 0xAA) && (MasterSync == 0)) //sunc vs master
		{
//			SetTimerTaskInfin(T_HeartBit, 0, 100);
			ClearTimerTask(RadioRead_T); //Stop listening
		}
		//Radio_MsgGet(&pack, data_buff);
		ClearBuf(&Radio_RxBuff);
	}
	__enable_irq();
}

DECLARE_TASK(RadioBroadcast_T)
{
	uint8_t data_buff[8];
	for(int i = 0; i<8; i++){data_buff[i]=0;}	
	if(IsMaster)
	{
		Radio_MsgSend(0xAA, data_buff);
	}
	else
	{
//		data_buff[0] = dustLvl;
		//data_buff[1] = (uint8_t)internalTemp;
		//data_buff[2] = (uint8_t)(internalTemp*100)>>2;
//		data_buff[1] = dh_T;
//		data_buff[2] = dh_H;
		
		Radio_MsgSend(0xBB, data_buff);
	}
}

void RADIO_IRQHandler(void)
{
	char t = 0;
  if(USART_GetITStatus(RADIO_UART, USART_IT_RXNE) != RESET)
  {
			t = USART_ReceiveData(RADIO_UART);
			WriteByte(&RX_BUFF, t);
  }
	if(USART_GetITStatus(RADIO_UART, USART_IT_TC) != RESET)
  {
    USART_ClearITPendingBit(RADIO_UART, USART_IT_TC);//очищаем признак прерывания
    if(!IsEmpty(&TX_BUFF))
		{
			USART_SendData(RADIO_UART,ReadByte(&TX_BUFF));
		}
		else
		{
			USART_ITConfig(RADIO_UART, USART_IT_TC, DISABLE);
		}
  }
}
