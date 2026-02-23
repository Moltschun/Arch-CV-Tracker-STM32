#include "sys.core.h"

void RCC_System_72HZ(void) {
    // Включение внешнего кварцевого резонатора (HSE)
    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));

    // Настройка задержки Flash памяти (необходима для работы на 72МГц)
    FLASH->ACR |= FLASH_ACR_PRFTBE;
    FLASH->ACR &= ~FLASH_ACR_LATENCY; 
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    // Конфигурация делителей шин AHB, APB1, APB2
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;  // AHB = 72МГц
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; // APB1 = 36МГц (max)
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1; // APB2 = 72МГц

    // Настройка PLL: умножение HSE (8МГц) на 9 = 72МГц
    RCC->CFGR &= ~(RCC_CFGR_PLLMULL | RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE);
    RCC->CFGR |= (RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC);

    // Запуск PLL и ожидание стабилизации
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    // Переключение системного тактирования на выход PLL
    RCC->CFGR &= ~(RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
}