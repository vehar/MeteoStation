#ifndef _INIT_H_
#define _INIT_H_

#include "macros.h"

//#include "misc.h"
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

	
#define PB_7  B, 7, HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ		
#define LED1  B, 0, HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ	
#define LED2  B, 1, HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ	

#define BOARD_LED  A, 5, HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ	

#define USER_KEY_A  C, 13, HIGH, MODE_INPUT_FLOATING, SPEED_2MHZ		
#define USER_KEY_B  B, 2, HIGH, MODE_INPUT_FLOATING, SPEED_2MHZ	

#define USART1_TX A, 9,  HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ	//MODE_AF_PUSH_PULL
#define USART1_RX A, 10, HIGH, MODE_INPUT_FLOATING, SPEED_2MHZ
	
#define PIN_LED					C, 9, HIGH, MODE_OUTPUT_PUSH_PULL, SPEED_2MHZ
#define PIN_BUTTON				A, 0, HIGH, MODE_INPUT_FLOATING, SPEED_2MHZ
#define VIBRO_SENSOR_PIN		C, 0, HIGH, MODE_INPUT_FLOATING, SPEED_2MHZ


void Adc_Init();
void Spi_Init(void);
void GPIO_Configuration(void);
void SysTikConfig(void);
void NVIC_Configuration(void);
void TIM_Configuration(void);
void USART_Configuration(void);

#endif //_INIT_H_