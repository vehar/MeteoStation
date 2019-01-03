#ifndef _INIT_H_
#define _INIT_H_

#include "macros.h"

#include "misc.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_i2c.h"
#include <stdio.h>
#include "DS1822.h"
#include "serial.h"
#include "systick.h"
#include "GLCD.h"
#include "strings.h"

#include "pinmap.h"


void Adc_Init();
void ADC_DMA_init();
void Spi_Init(void);
void GPIO_Configuration(void);
void SysTikConfig(void);
void NVIC_Configuration(void);
void TIM_Configuration(void);
void USART_Configuration(void);

#endif //_INIT_H_