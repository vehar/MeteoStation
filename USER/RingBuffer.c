// RingBufferTest.cpp : Defines the entry point for the console application.
//
#include <RingBuffer.h>

// ClearBuf очищает буфер (может также использоваться для инициализации структуры CircularBuffer)
void ClearBuf(CircularBuffer* pBuf)
{
  pBuf->first = 0;
  pBuf->last = 0;
}

// ReadByte читает байт из буфера.  если в буфере нет данных, возвращает -1.
uint8_t ReadByte(CircularBuffer* pBuf)
{
  if (IsEmpty(pBuf))
    return -1;
  int result = pBuf->data[pBuf->first];
  pBuf->first = (pBuf->first + 1) & (BUFFER_SIZE - 1);
  return result;
}

// пишет байт в буфер, возвращает true если запись прошла успешно
bool WriteByte(CircularBuffer* pBuf, uint8_t value)
{
  size_t next = (pBuf->last + 1) & (BUFFER_SIZE - 1);
  if (next == pBuf->first)
    return false;
  pBuf->data[pBuf->last] = value;
  pBuf->last = next;
  return true;
}

// функция IsEmpty возвращает true если буфер пуст, иначе false
// пустым являтся буфер в котором нет данных для чтения.
bool IsEmpty(CircularBuffer* pBuf)
{
	return (pBuf->first == pBuf->last);
}

// функция IsFull возвращает true если буфер полон, иначе false
// попытка писать в полный буфер всегда будет завершаться неудачей.
bool IsFull(CircularBuffer* pBuf)
{
  return GetAmount(pBuf) == (BUFFER_SIZE - 1);
}

// что возвращает функция GetSomething? переименуйте ее, чтобы название соответствоало возвращаемому значению 
size_t GetAmount(CircularBuffer* pBuf)
{
  return (pBuf->last - pBuf->first) & (BUFFER_SIZE - 1);
}

size_t GetFree(CircularBuffer* pBuf)
{
  return (BUFFER_SIZE - 1) - GetAmount(pBuf);
}

size_t BufMoveFast(CircularBuffer* pDest, CircularBuffer* pSource)
{
	int amountSrc = 0;
	int freeDest = 0;
	int destEndChunk = 0;
	int numElems = 0;
	int copiedAmount = 0;
	int srcLinedChunk = 0;
	int dstLinedChunk = 0;

	while(!IsFull(pDest) && !IsEmpty(pSource))
	{
		 amountSrc = GetAmount(pSource);
		 freeDest = GetFree(pDest);
		 srcLinedChunk = (BUFFER_SIZE - pSource->first);
		 dstLinedChunk = (BUFFER_SIZE - pDest->last);
		 numElems = 0;

		numElems = MIN(MIN(amountSrc,freeDest), MIN(srcLinedChunk,dstLinedChunk));

		memcpy(pDest->data+pDest->last, pSource->data+pSource->first, numElems);

		pDest->last = (pDest->last + numElems) & (BUFFER_SIZE - 1);
		pSource->first = (pSource->first + numElems) & (BUFFER_SIZE - 1);

		copiedAmount += numElems;
	}
	return copiedAmount;
}

// вспомогательная функция для отладки
void PrintBuffer(CircularBuffer* pBuf) 
{   
  if (pBuf->first == pBuf->last){ printf(" Empty");}
  for (size_t pos = pBuf->first; pos != pBuf->last; pos = (pos + 1) & (BUFFER_SIZE - 1))
    printf(" %02x", pBuf->data[pos]);
  printf("\n");
}

/*
CircularBuffer bufferA;
CircularBuffer bufferB;

int full = 0;
int free = 0;

int main(){
    ClearBuf(&bufferA);
        ClearBuf(&bufferB);
// another exemple 
  
  
        WriteByte(&bufferA, 1);
        WriteByte(&bufferA, 2);
        WriteByte(&bufferA, 3);
        WriteByte(&bufferA, 4);
        WriteByte(&bufferA, 5);

  
      // write six zeros
      for(int i = 0; i < 6; ++i)
          WriteByte(&bufferB, i*10);

      
      // read six zeros
      for(int i = 0; i < 6; ++i)
          ReadByte(&bufferB);
      
      // so it is empty now
          
  
printf("BufferA IsEmpty: %d \r\n", IsEmpty(&bufferA));
    printf("BufferA IsFull: %d \r\n", IsFull(&bufferA));

        WriteByte(&bufferA, 4);
        WriteByte(&bufferA, 5);

    printf("BufferA IsEmpty: %d \r\n", IsEmpty(&bufferA));
    printf("BufferA IsFull: %d \r\n", IsFull(&bufferA));

    ReadByte(&bufferA);
    for(int i = 0; i < BUFFER_SIZE; i++){WriteByte(&bufferA, i); full = GetAmount(&bufferA); free = GetFree(&bufferA);}

    printf("BufferA IsEmpty: %d \r\n", IsEmpty(&bufferA));
    printf("BufferA IsFull: %d \r\n", IsFull(&bufferA));

        WriteByte(&bufferB, 1);
        WriteByte(&bufferB, 2);
        WriteByte(&bufferB, 3);

// ---- Added two lines
  
        ReadByte(&bufferB);
        ReadByte(&bufferB);
// ----- --- ---- 
  
        printf("BufferA before move:");
        PrintBuffer(&bufferA);
        printf("BufferB before move:");
        PrintBuffer(&bufferB);

        //size_t res = BufMoveSlow(&bufferB, &bufferA);//BufMoveSlow
        //printf("BufMoveSlow moved %zu item(s) from BufferA to BufferB\n", res);//BufMoveSlow

        size_t res = BufMoveFast(&bufferB, &bufferA);//BufMoveSlow
        printf("BufMoveFast moved %zu item(s) from BufferA to BufferB\n", res);//BufMoveSlow

        printf("BufferA after move:");
        PrintBuffer(&bufferA);
        printf("BufferB after move:");
        PrintBuffer(&bufferB);
  
  return 0;
}*/