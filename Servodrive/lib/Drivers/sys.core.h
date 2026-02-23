/**
 * @file sys.core.h
 * @brief Конфигурация тактирования системы (RCC).
 */

#ifndef SYS_CORE_H_
#define SYS_CORE_H|

#include "stm32f1xx.h"

/**
 * @brief Настройка системы на максимальную частоту 72 МГц через PLL (HSE).
 */
void RCC_System_72HZ(void);

#endif /* SYS_CORE_H_ */