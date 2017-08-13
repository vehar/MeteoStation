#include "1-Wire.h"
#include "..\platform.h"
#include "macros.h"

#include "stm32_GPIO.h"
#include "systick.h"

unsigned int One_Wire_Reset(GPIO_TypeDef * GPIOx, u16 PINx)								
{
	unsigned int tmp;
	Pin_In(GPIOx, PINx);
	if ((Pin_Syg(GPIOx, PINx))==0)	return One_Wire_Bus_Low_Error;
	Pin_Out_PP(GPIOx, PINx);
	Pin_Off(GPIOx, PINx);
	delay_us(Time_Reset_Low);
	Pin_On(GPIOx, PINx);
	Pin_In(GPIOx, PINx);
	delay_us(Time_Pulse_Delay_High);
	if ((Pin_Syg(GPIOx, PINx))==0) tmp=One_Wire_Success;
		else tmp=One_Wire_Error_No_Echo;
	delay_us(Time_After_Reset);
	return tmp;
}

void One_Wire_Write_Byte(unsigned char Byte,GPIO_TypeDef * GPIOx, u16 PINx)
{
	unsigned char cnt;
	for (cnt=0;cnt!=8;cnt++) One_Wire_Write_Bit(Byte&(1<<cnt),GPIOx, PINx);
}

void One_Wire_Write_Bit (unsigned char Bit,GPIO_TypeDef * GPIOx, u16 PINx)
{
	Pin_Out_PP(GPIOx, PINx);
	Pin_Off(GPIOx, PINx);
	if (Bit==0)
	{
		delay_us(Time_Pulse_Delay_High);
		Pin_On(GPIOx, PINx);
		delay_us(Time_Pulse_Delay_Low);
	}
	else
	{
		delay_us(Time_Pulse_Delay_Low);
		Pin_On(GPIOx, PINx);
		delay_us(Time_Pulse_Delay_High);
	}
	Pin_In(GPIOx, PINx);
} 

unsigned char One_Wire_Read_Byte(GPIO_TypeDef * GPIOx, u16 PINx)
{
	unsigned char tmp=0;
	unsigned char cnt;
	for (cnt=0;cnt!=8;cnt++)
		if (One_Wire_Read_Bit(GPIOx, PINx)!=0)	tmp|=(1<<cnt);
	delay_us(Time_Pulse_Delay_High);
	return tmp;
}

unsigned char One_Wire_Read_Bit (GPIO_TypeDef * GPIOx, u16 PINx)
{
		unsigned char tmp;
	 	Pin_Out_PP(GPIOx, PINx);
		Pin_Off(GPIOx, PINx);
		delay_us(Time_Hold_Down);
		Pin_In(GPIOx, PINx);
		delay_us(Time_Pulse_Delay_Low);
		if ((Pin_Syg(GPIOx, PINx))!=0)	tmp = 1;
			else tmp = 0;
		delay_us(Time_Pulse_Delay_High);
		return tmp;
}
