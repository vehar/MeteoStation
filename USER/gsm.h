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

//#define GENERATE_ARR(NAM,...) char arr_##NAM[]=__VA_ARGS__

#define CMDR(cmd,...)         char _##cmd[]=#cmd###__VA_ARGS__
#define CMD(cmd)         char _##cmd[]=#cmd

#define ARG(cmd,args)			_##cmd##args
	
extern RingBuffer Gsm_RxBuff;
extern RingBuffer Gsm_TxBuff;

enum GSM_state{ON, OFF, OFFLINE};

extern uint8_t GSM_state_f;

void AT_GSM_MsgSend(char* data_buff, uint8_t sz);
	
int GSM_MsgGet(char* data_buff);	
void AT_GSM_MsgSendV(char* data_buff, ...);
void GSM_MsgSend(char* data_buff, uint8_t sz);

DECLARE_TASK(GSM_FTP_Connect);
DECLARE_TASK(GSM_Actions);
DECLARE_TASK(GSM_Ping);
DECLARE_TASK(GSM_Call);
DECLARE_TASK(GSM_On);

DECLARE_TASK(GSM_Lib);


#endif //_GSM_H_
