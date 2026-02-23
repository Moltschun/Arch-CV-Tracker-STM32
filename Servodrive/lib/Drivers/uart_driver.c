#include "uart_driver.h"

volatile uint32_t rx_buffer = 0;
volatile uint32_t rx_flag = 0;

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
        rx_buffer = USART1->DR;
        rx_flag = 1; // Установка программного флага
    }
}