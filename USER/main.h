/*
 * File: main.c
 * Date: 31.08.2012
 * Denis Zheleznyakov http://mcu.ziblog.ru
 */

#ifndef MAIN_H_
#define MAIN_H_
 
#include "macros.h"

//#include "misc.h"

#include <stdio.h>
#include "DS1822.h"
#include "serial.h"
#include "systick.h"
#include "GLCD.h"
#include "strings.h"
#include "dht11.h"

#include "..\Libraries\platform.h"

#define One_Wire_Pin 		GPIOB, GPIO_Pin_9

//////////
#define	 CS_LOW 	GPIOB->BSRR = GPIO_BSRR_BR1;
#define  CS_HIGH 	GPIOB->BSRR = GPIO_BSRR_BS1;

//для SWO
#define 	 ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define 	 ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define 	 ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

#define    DWT_CYCCNT    *(volatile unsigned long *)0xE0001004
#define    DWT_CONTROL   *(volatile unsigned long *)0xE0001000
#define    SCB_DEMCR     *(volatile unsigned long *)0xE000EDFC
	
struct __FILE { int handle;  };

/* Private function prototypes -----------------------------------------------*/

	
unsigned char One_Wire_Error_Handle (unsigned char err);


#endif /* MAIN_H_ */
