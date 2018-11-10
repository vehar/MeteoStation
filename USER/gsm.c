#include "gsm.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "Sim80x.h"

#include "debug.h"

uint8_t GSM_state_f = OFFLINE;
RingBuffer Gsm_RxBuff;
RingBuffer Gsm_TxBuff;

DECLARE_TASK(GSM_LibInit);
DECLARE_TASK(GSM_Lib)
{
	SetTimerTaskInfin(StartSim80xTask, 0, 100);
	SetTimerTaskInfin(StartSim80xBuffTask, 0, 10);
	SetTimerTaskInfin(GSM_LibInit, 100, 0);
}

DECLARE_TASK(GSM_LibInit)
{
	Sim80x_Init(1);
	Sim80x_InitValue();
}

DECLARE_TASK(GSM_On)
{
	DEBUGFMSG_START(); 
	TaskSuspend(GSM_Ping);
	if(GSM_state_f == OFF)
	{
		PIN_OFF(GSM_ON_OFF);
		DEBUGFMSG("PIN_OFF\r"); 
		T_Delay(3000);
		PIN_ON(GSM_ON_OFF);
		DEBUGFMSG("PIN_ON\r"); 
		T_Delay(500);
	}		
	TaskResume(GSM_Ping);
}

DECLARE_TASK(GSM_Call)
{
	uint8_t AT[] = "ATD+380930893448;\r";
	DEBUGFMSG("GSM_Call\r\n");
//	GSM_MsgSend(AT, sizeof(AT));
	TaskSuspend(GSM_Call);
}


DECLARE_TASK(GSM_Ping)
{
	bool exit = false;
	uint8_t AT[] = {"AT\r"};
	uint8_t msgBuff[50];
	memset(msgBuff, 0, 50);

		DEBUGFMSG("..\r\n"); 
	
		GSM_MsgSend(AT, sizeof(AT));   //ATOMIC BUFFER
		delay_ms(500);								 //ATOMIC BUFFER!!!
		GSM_MsgGet(msgBuff);					 //ATOMIC BUFFER
	
		if(strstr(msgBuff, "OK") != 0) //GSM - online and ON
		{
			GSM_state_f = ON; 
			printf("GSM_OK\r\n"); 
			
			TaskResume(GSM_Actions);
			SetTimerTaskInfin(GSM_Actions, 0, 1000);
			
			ClearTimerTask(GSM_Ping);
			return;
		} 
		else if (strstr(msgBuff, "AT") != 0) //GSM - online but OFF
		{
			GSM_state_f = OFF; 
		} 
		else //GSM - offline
		{
			GSM_state_f = OFFLINE;
		} 
		DEBUGMSG("%s \r\n",msgBuff); 
		memset(msgBuff, 0, 50);	
	
	if(GSM_state_f != OFFLINE) 
	{
			SetTimerTaskInfin(GSM_On, 500, 0);
	}

//	GSM_MsgSend("ATD+380930893448;\r\n"); 
}


DECLARE_TASK(GSM_Actions)
{
	if(GSM_state_f == ON)
	{
		DEBUGFMSG("OK\r\n"); 
	}
	else
	{
		DEBUGFMSG("FAIL\r\n"); 
		TaskSuspend(GSM_Actions);
		SetTimerTaskInfin(GSM_Ping, 0, 1000);
	}
}

CMD(CSQ);
CMD(CREG);
//If +CREG: 0,1 //The device is registered in home network.

//uint8_t C1[] = {"AT+SAPBR=3,1,"CONTYPE","GPRS""};
//AT+SAPBR=3,1,"APN","internet"
//AT+SAPBR=3,1,"USER",""
//AT+SAPBR=3,1,"PWD",""

////Open Bearer 1 once
//AT+SAPBR=1,1 - On start once
//Wait for the bearer to be opened
//WAIT=6

//Set the the CID for FTP session
//AT+FTPCID=1

//Set the FTP server name
//AT+FTPSERV="simcom.exavault.com"

//Set the FTP user name
//AT+FTPUN="zyf"

//Set the FTP password
//AT+FTPPW="zyf"

//Set the FTP filename to get
//AT+FTPGETNAME="12.txt"

//Set the FTP directory
//AT+FTPGETPATH="/"

//Perform a FTP get
//AT+FTPGET=1

//Wait
//WAIT till +FTPGET: 1,1

//Get the data
//AT+FTPGET=2,1024

DECLARE_TASK(GSM_FTP_Connect)
{
	DEBUGFMSG(""); 

	uint8_t CSQ[] = {"CSQ"};
	
	AT_GSM_MsgSend(_CREG, sizeof(_CREG));
}

char buff[60];

void AT_GSM_MsgSendV(char* data_buff, ...)
{
	char * arrayPtr = (char*) calloc(50 ,sizeof(char)); 
	va_list argptr;
	va_start (argptr, data_buff);
	
	sprintf(buff, "AT+%s=%s\r\n",va_arg(argptr, char*),va_arg(argptr, char*));
	
	va_end(argptr);
}

void AT_GSM_MsgSend(char* data_buff, uint8_t sz)
{
	char AT[] = {"AT+"};
	char * arrayPtr = (char*) calloc(50 ,sizeof(char)); 
	strcat(arrayPtr, AT);
	strcat(arrayPtr, (char*)data_buff);
	memcpy(buff, arrayPtr, sz+3);
	DEBUGMSG("strcat %s", arrayPtr); 
	
	GSM_MsgSend(arrayPtr, sz+3);
	free (arrayPtr);
}

void GSM_MsgSend(char* data_buff, uint8_t sz)
{
	for(int i = 0; i<sz; i++)
	{
		WriteByte(&GSM_TX_BUFF, data_buff[i]);
	}
	
	USART_SendData(GSM_UART, ReadByte(&GSM_TX_BUFF));
	USART_ITConfig(GSM_UART, USART_IT_TC, ENABLE);
}

int GSM_MsgGet(char* data_buff)
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
			Sim80x_RxCallBack(t);
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