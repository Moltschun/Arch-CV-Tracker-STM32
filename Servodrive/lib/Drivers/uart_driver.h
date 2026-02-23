#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

#include "stm32f1xx.h"

extern volatile uint32_t rx_buffer;
extern volatile uint32_t rx_flag;


void UART_Init(void);

void UART_SendChar(char symbol);

void UART_String(char *string);

#endif