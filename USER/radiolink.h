//radiolink.h
#pragma once
#include <stdbool.h>
#include <stm32f10x.h>
#include <stm32f10x_dma.h>
#include "stm32f10x_usart.h"
#include "RingBuffer.h"

extern CircularBuffer RxBuff;
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
int MsgSend(uint32_t msgId, uint8_t* data_buff);
int MsgGet(RadioPack_t* pack, uint8_t* data_buff);
