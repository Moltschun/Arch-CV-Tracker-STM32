#include "uart_driver.h"

volatile uint32_t rx_buffer = 0;
volatile uint32_t rx_flag = 0;

void UART_Init(void){

    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    GPIOA->CRH &= ~(0xFF0);

    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9;

    USART1->BRR = 0x0271;

    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;


    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);
}

void UART_SendChar(char symbol){

    while(!(USART1->SR & USART_SR_TXE));

    USART1->DR = symbol;
}

void UART_String(char *string){

    while (*string)
    {
        UART_SendChar(*string++);
    }
    
}

void USART1_IRQHandler(void){

    if (USART1->SR & USART_SR_RXNE){

        rx_buffer = USART1->DR;

        rx_flag = 1;
    }
}

