#include "radiolink.h"
#include "RingBuffer.h"

#include <stdio.h>
#include <string.h>

RingBuffer Radio_RxBuff;
RingBuffer Radio_TxBuff;

struct u_id HostID;

#define MMIO16(addr)  (*(volatile uint16_t *)(addr))
#define MMIO32(addr)  (*(volatile uint32_t *)(addr))
#define U_ID          0x1ffff7e8

/* Read U_ID register */
void uid_read(struct u_id *id)
{
    id->off0 = MMIO16(U_ID + 0x0);
    id->off2 = MMIO16(U_ID + 0x2);
    id->off4 = MMIO32(U_ID + 0x4);
    id->off8 = MMIO32(U_ID + 0x8);
}

/* Returns true if IDs are the same */
bool uid_cmp(struct u_id *id1, struct u_id *id2)
{
    return id1->off0 == id2->off0 &&
           id1->off2 == id2->off2 &&
           id1->off4 == id2->off4 &&
           id1->off8 == id2->off8;
}

/*
    struct u_id id1 = { 0x0, 0x1, 0x2, 0x3 };
    struct u_id id2;
    bool same_id;

    uid_read(&id2);
    same_id = uid_cmp(&id1, &id2);

    printf("%s\n", same_id ? "equal" : "not equal");
*/

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
