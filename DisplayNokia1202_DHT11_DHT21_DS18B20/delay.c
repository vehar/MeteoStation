#include "delay.h"

void delay_ms(volatile int i)
{
	i *= 1000; 
	while (i) i--;
}

void delay_us(volatile int i)
{
	i *= 1; 
	while (i) i--;
}
