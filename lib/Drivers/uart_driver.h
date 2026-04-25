/**
 * @file uart_driver.h
 * @brief Драйвер интерфейса UART для STM32F103 (CMSIS).
 * @author Pilot
 */

#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

#include "stm32f1xx.h"

/* Глобальные флаги обмена данными */
extern volatile char rx_buffer[32]; ///< Буфер последнего принятого байта
extern volatile uint8_t rx_flag;   ///< Флаг готовности данных (1 - получено, 0 - пусто)
extern volatile uint8_t rx_index;

/**
 * @brief Инициализация USART1 (115200 baud, 8N1, прерывания RX).
 */
void UART_Init(void);

/**
 * @brief Передача одного символа.
 * @param symbol Символ для отправки.
 */
void UART_SendChar(char symbol);

/**
 * @brief Передача строки.
 * @param string Указатель на null-terminated строку.
 */
void UART_String(char *string);

#endif /* UART_DRIVER_H_ */