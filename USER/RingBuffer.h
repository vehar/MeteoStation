//RingBuffer.h

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))

#define uint8_t unsigned char

// дан циклический буфер и некоторые функции работы с ним
#define BUFFER_SIZE 512

#if (BUFFER_SIZE & (BUFFER_SIZE - 1)) != 0
  #error "Incorrect buffer size"
#endif

typedef struct {
  size_t first;
  size_t last;
  uint8_t data[BUFFER_SIZE];
} CircularBuffer;

size_t GetAmount(CircularBuffer* pBuf);
bool IsFull(CircularBuffer* pBuf);
bool IsEmpty(CircularBuffer* pBuf);
void ClearBuf(CircularBuffer* pBuf);
uint8_t ReadByte(CircularBuffer* pBuf);
bool WriteByte(CircularBuffer* pBuf, uint8_t value);
void PrintBuffer(CircularBuffer* pBuf);
size_t BufMoveFast(CircularBuffer* pDest, CircularBuffer* pSource);
size_t GetFree(CircularBuffer* pBuf);
size_t GetAmount(CircularBuffer* pBuf);
