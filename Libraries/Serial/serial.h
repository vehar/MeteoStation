#include "stm32f10x_usart.h"
void uart_send_char (USART_TypeDef * USARTx, char dat);
void uart_print_string(USART_TypeDef * USARTx, char * string, char newline);
void uart_print_value(USART_TypeDef * USARTx, long val);
void uart_print_hex_value(USART_TypeDef * USARTx, long val);
