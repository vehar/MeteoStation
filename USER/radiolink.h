//radiolink.h
#pragma once
#include <stdbool.h>
#include <stm32f10x.h>
#include <stm32f10x_dma.h>
#include "stm32f10x_usart.h"
#include "RingBuffer.h"
#include "init.h"

#define RADIO_UART USART1
#define RADIO_IRQHandler USART1_IRQHandler
#define TX_BUFF Radio_TxBuff
#define RX_BUFF Radio_RxBuff

extern RingBuffer Radio_RxBuff;
extern RingBuffer Radio_TxBuff;
extern struct u_id HostID;

struct u_id {
    uint16_t off0;
    uint16_t off2;
    uint32_t off4;
    uint32_t off8;
};

typedef struct {
  struct u_id senderId;
  uint8_t data[8];
  uint8_t msgId;
  uint8_t crc;
} RadioPack_t;

extern bool uid_cmp(struct u_id *id1, struct u_id *id2);
extern void uid_read(struct u_id *id);
int Radio_MsgSend(uint32_t msgId, uint8_t* data_buff);
int Radio_MsgGet(RadioPack_t* pack, uint8_t* data_buff);

int RadioB_MsgGet(uint8_t* data_buff);
void RadioB_MsgSend(uint8_t* data_buff, uint8_t sz);
void HC12_configBaud(int baud);