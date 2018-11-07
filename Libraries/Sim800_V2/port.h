//port.h
#ifndef _PORT_H_
#define _PORT_H_

#include "EERTOS.h"
#include "gsm.h"

#define _SIM80X_POWER_KEY_GPIO (1)
#define _SIM80X_POWER_KEY_PIN (2)
#define GPIO_PIN_SET (3)
#define GPIO_PIN_RESET (4)

void HAL_GPIO_WritePin(uint8_t PORT,uint8_t PIN, uint8_t mode);
	
typedef int osPriority;
typedef int osThreadId;

uint32_t HAL_GetTick();
void	Sim80x_SendString(char *str);
void  Sim80x_SendRaw(uint8_t *Data,uint16_t len);

#endif //_PORT_H_