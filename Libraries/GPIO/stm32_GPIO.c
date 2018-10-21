#include "stm32_GPIO.h"
#include "stm32f10x_gpio.h"

void Pin_On(GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIOx->BSRR=PINx;
	//GPIOx->ODR=GPIOx->IDR|(PINx);
}

void Pin_Off(GPIO_TypeDef * GPIOx,u16 PINx)								
{
	GPIOx->BRR=PINx;
	//GPIOx->ODR=GPIOx->IDR&(~(PINx));
}

u8 Pin_Syg (GPIO_TypeDef * GPIOx, u16 PINx)
{
	if((GPIOx->IDR&PINx)!=0)
	{return 1;}
	else
	{return 0;}
}

void Pin_In (GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=PINx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;//GPIO_Mode_IN_FLOATING
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Pin_Out_PP (GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=PINx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Pin_Out_Od (GPIO_TypeDef * GPIOx,u16 PINx)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin=PINx;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Pin_Inv(GPIO_TypeDef * GPIOx, u16 PINx)
{
	if(Pin_Syg(GPIOx, PINx)!=0)
	{Pin_Off(GPIOx, PINx);}
	else
	{Pin_On(GPIOx, PINx);}
}

