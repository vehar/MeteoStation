#include "port.h"
#include "gsm.h"

uint32_t HAL_GetTick()
{
	return v_u32_SYS_TICK;
}

void osDelay(uint32_t delay_ticks)
{
	T_Delay(delay_ticks);
}

void	Sim80x_SendString(char *str)
{
	GSM_MsgSend(str, sizeof(str));// TODO: check size!
}

void  Sim80x_SendRaw(uint8_t *Data, uint16_t len)
{
	GSM_MsgSend(Data, len);
}


void HAL_GPIO_WritePin(uint8_t PORT,uint8_t PIN, uint8_t mode)
{
	if(mode == GPIO_PIN_RESET)
	{
		PIN_OFF(GSM_ON_OFF);
	}
	else
	{
		PIN_ON(GSM_ON_OFF);
	}
}