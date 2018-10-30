#include "port.h"
#include "gsm.h"

int HAL_GetTick()
{
	return 1;
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