#include "servo.h"

void Servo_Init(void){

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;

    GPIOA->CRL &= ~(0xFF);
    GPIOA->CRL |= GPIO_CRL_MODE0 | GPIO_CRL_CNF0_1;
    GPIOA->CRL |= GPIO_CRL_MODE1 | GPIO_CRL_CNF1_1;

    TIM2->PSC = 71;
    TIM2->ARR = 19999;

    
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= (0x6 << 4);

    TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM2->CCMR1 |= (0x6 << 12);

    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

    
    TIM2->CCR1 = 1500;
    TIM2->CCR2 = 1500;
    
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR |= TIM_EGR_UG;

    TIM2->CR1 |= TIM_CR1_CEN;
}

void Servo_Turn (uint8_t channel, uint32_t degree){

    if (degree > 180) degree = 180;

    uint32_t turn = 500 + (degree * 2000) / 180;

    if (channel == 1) TIM2->CCR1 = turn;
    else if (channel == 2) TIM2->CCR2 = turn;
}

