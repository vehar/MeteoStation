//RingBuffer.h
#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

#define uint8_t unsigned char

// дан циклический буфер и некоторые функции работы с ним
#define BUFFER_SIZE 64

#if (BUFFER_SIZE & (BUFFER_SIZE - 1)) != 0
  #error "Incorrect buffer size"
#endif

typedef struct {
  size_t first;
  size_t last;
  uint8_t data[BUFFER_SIZE];
} RingBuffer;

extern size_t GetAmount(RingBuffer* pBuf);
extern bool IsFull(RingBuffer* pBuf);
extern bool IsEmpty(RingBuffer* pBuf);
extern void ClearBuf(RingBuffer* pBuf);
extern uint8_t ReadByte(RingBuffer* pBuf);
extern bool WriteByte(RingBuffer* pBuf, uint8_t value);
extern void PrintBuffer(RingBuffer* pBuf);
extern size_t BufMoveFast(RingBuffer* pDest, RingBuffer* pSource);
extern size_t GetFree(RingBuffer* pBuf);
extern size_t GetAmount(RingBuffer* pBuf);
