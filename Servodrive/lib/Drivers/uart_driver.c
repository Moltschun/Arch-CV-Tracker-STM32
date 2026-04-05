#include "uart_driver.h"

volatile char rx_buffer[32];
volatile uint8_t rx_flag = 0;
volatile uint8_t rx_index = 0;

void UART_Init(void) {
    // Включение тактирования GPIOA и USART1
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    // Настройка выводов: PA9 (TX) - Alt. Push-Pull, PA10 (RX) - Input Floating
    GPIOA->CRH &= ~(0xFF0);
    GPIOA->CRH |= GPIO_CRH_CNF10_0;                   // RX: Floating input
    GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9;   // TX: Alt. output 50MHz

    // Расчет BRR для 115200 при f_PCLK2 = 72MHz
    USART1->BRR = 0x0271; 

    // Включение приемопередатчика и прерывания по приему
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;

    // Настройка NVIC для обработки прерываний USART1
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);
}

void UART_SendChar(char symbol) {
    while(!(USART1->SR & USART_SR_TXE));
    USART1->DR = symbol;
}

void UART_String(char *string) {
    while (*string) UART_SendChar(*string++);
}

/**
 * @brief Обработчик прерывания USART1.
 * Вызывается аппаратно при получении байта.
 */
void USART1_IRQHandler(void) {
    if (USART1->SR & USART_SR_RXNE) {
        char data = USART1->DR;
        
        if (data == '\n') {
            rx_buffer[rx_index] = '\0'; // Закрываем строку
            rx_flag = 1;           // Поднимаем флаг для main
            rx_index = 0;               // Сбрасываем указатель для нового пакета
        } 
        // Игнорируем символ возврата каретки, если он есть
        else if (data != '\r' && rx_index < sizeof(rx_buffer) - 1) {
            rx_buffer[rx_index++] = data;
        } 
        else if (rx_index >= sizeof(rx_buffer) - 1) {
            rx_index = 0; // Защита от переполнения (сброс мусора)
        }
    }
}