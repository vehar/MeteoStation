//port.h
#ifndef _PORT_H_
#define _PORT_H_

#include "EERTOS.h"
#include "gsm.h"

#define HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_RESET) PIN_OFF(GSM_ON_OFF)
#define HAL_GPIO_WritePin(_SIM80X_POWER_KEY_GPIO,_SIM80X_POWER_KEY_PIN,GPIO_PIN_SET) PIN_ON(GSM_ON_OFF)

typedef int osPriority;
typedef int osThreadId;

int HAL_GetTick();
void	Sim80x_SendString(char *str);
void  Sim80x_SendRaw(uint8_t *Data,uint16_t len);

#endif //_PORT_H_