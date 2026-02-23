/**
 * @file servo.h
 * @brief Драйвер управления сервоприводами через PWM (TIM2).
 */

#ifndef SERVO_H_
#define SERVO_H_

#include "stm32f1xx.h"

/**
 * @brief Инициализация таймера TIM2 для генерации ШИМ 50Гц.
 */
void Servo_Init(void);

/**
 * @brief Поворот сервопривода на заданный угол.
 * @param channel Номер канала (1 или 2).
 * @param degree Угол в градусах (0-180).
 */
void Servo_Turn(uint8_t channel, uint32_t degree);

#endif /* SERVO_H_ */