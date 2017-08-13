#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

void Pin_On(GPIO_TypeDef * GPIOx,u16 PINx);
void Pin_Off(GPIO_TypeDef * GPIOx,u16 PINx);		
u8 Pin_Syg (GPIO_TypeDef * GPIOx, u16 PINx);
void Pin_In (GPIO_TypeDef * GPIOx,u16 PINx);
void Pin_Out_PP (GPIO_TypeDef * GPIOx,u16 PINx);
void Pin_Out_Od (GPIO_TypeDef * GPIOx,u16 PINx);
void Pin_Inv(GPIO_TypeDef * GPIOx, u16 PINx);

