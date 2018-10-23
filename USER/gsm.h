//gsm.h
#ifndef _GSM_H_
#define _GSM_H_

#include <stm32f10x.h>
#include <stm32f10x_dma.h>
#include "stm32f10x_usart.h"
#include "macros.h"
#include "init.h"
#include "RingBuffer.h"
#include "EERTOS.h"

#define GSM_UART USART3
#define GSM_IRQHandler USART3_IRQHandler
#define GSM_TX_BUFF Gsm_TxBuff
#define GSM_RX_BUFF Gsm_RxBuff

extern RingBuffer Gsm_RxBuff;
extern RingBuffer Gsm_TxBuff;

enum GSM_state{ON, OFF, OFFLINE};

extern uint8_t GSM_state_f;

int GSM_MsgGet(uint8_t* data_buff);	
void GSM_MsgSend(uint8_t* data_buff, uint8_t sz);

DECLARE_TASK(GSM_Actions);
DECLARE_TASK(GSM_Ping);
DECLARE_TASK(GSM_Call);
DECLARE_TASK(GSM_On);

#endif //_GSM_H_
