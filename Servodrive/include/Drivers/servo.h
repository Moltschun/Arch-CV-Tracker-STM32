#ifndef SERVO_H_
#define SERVO_H_

#include "stm32f1xx.h"

void Servo_Init(void);

void Servo_Turn(uint8_t channel, uint32_t degree);

#endif