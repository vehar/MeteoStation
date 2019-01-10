#ifndef PMS_H_
#define PMS_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "RingBuffer.h"

#define PMS_UART USART2
#define PMS_IRQHandler USART2_IRQHandler
#define PMS_TX_BUFF Pms_TxBuff
#define PMS_RX_BUFF Pms_RxBuff

extern RingBuffer Pms_TxBuff;
extern RingBuffer Pms_RxBuff;

/*
void loop()
{
  if (pms.read(data))
  {
    Serial1.print("PM 1.0 (ug/m3): ");
    Serial1.println(data.PM_AE_UG_1_0);
    Serial1.print("PM 2.5 (ug/m3): ");
    Serial1.println(data.PM_AE_UG_2_5);
    Serial1.print("PM 10.0 (ug/m3): ");
    Serial1.println(data.PM_AE_UG_10_0);
  }
}
*/
#define MAKEWORD(a,b)   ((uint16_t)(((uint8_t)(a))|(((uint16_t)((uint8_t)(b)))<<8)))

typedef struct{
  // Standard Particles, CF=1
  uint16_t PM_SP_UG_1_0;
  uint16_t PM_SP_UG_2_5;
  uint16_t PM_SP_UG_10_0;

  // Atmospheric environment
  uint16_t PM_AE_UG_1_0;
  uint16_t PM_AE_UG_2_5;
  uint16_t PM_AE_UG_10_0;
}PMS_DATA;


  enum STATUS { STATUS_WAITING, STATUS_OK };
  enum MODE { MODE_ACTIVE, MODE_PASSIVE };  
  
extern  uint8_t 	_status;
extern	uint8_t 	_mode;
extern  uint8_t 	_payload[12];
extern  uint8_t* 	_msgBuff;
  
extern  PMS_DATA* 		_data;
extern  uint8_t 	_index;
extern  uint16_t 	_frameLen;
extern  uint16_t 	_checksum;
extern  uint16_t 	_calculatedChecksum;
extern  bool pms_data_available; 	
	
	void PMS_Config();
	
  void sleep();
  void wakeUp();
  void activeMode();
  void passiveMode();

  void requestRead();
  bool PMS_read(PMS_DATA* data, uint8_t* msgBuff);
  bool readUntil(PMS_DATA* data, uint16_t timeout);

  bool loop(char ch);
  
#endif //PMS_H_