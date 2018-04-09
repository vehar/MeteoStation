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
} CircularBuffer;

extern size_t GetAmount(CircularBuffer* pBuf);
extern bool IsFull(CircularBuffer* pBuf);
extern bool IsEmpty(CircularBuffer* pBuf);
extern void ClearBuf(CircularBuffer* pBuf);
extern uint8_t ReadByte(CircularBuffer* pBuf);
extern bool WriteByte(CircularBuffer* pBuf, uint8_t value);
extern void PrintBuffer(CircularBuffer* pBuf);
extern size_t BufMoveFast(CircularBuffer* pDest, CircularBuffer* pSource);
extern size_t GetFree(CircularBuffer* pBuf);
extern size_t GetAmount(CircularBuffer* pBuf);
