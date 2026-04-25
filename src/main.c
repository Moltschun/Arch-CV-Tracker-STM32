#include "servo.h"
#include "uart_driver.h"
#include "sys.core.h"
#include <stdio.h>

// --- Настройки ПИД-регулятора ---
float Kp = 0.15f;
float Ki = 0.001f;
float Kd = 0.01f;

// --- Состояние системы ---
int prev_error_x = 0;
int integral_x = 0;
int prev_error_y = 0;
int integral_y = 0;

// Параметры для оси Y (180 градусов)
float tilt_angle = 90.0f; 

// Параметры для оси X (360 градусов - Keyestudio)
float x_neutral = 90.0f; // Точка полной остановки (калибруется)
float speed_k = 1.2f;    // Чувствительность скорости вращения

int main(void) {
    // Инициализация аппаратных уровней
    RCC_System_72HZ();
    UART_Init();
    Servo_Init();

    // Инициализация диагностического светодиода (PC13)
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0xF << 20);
    GPIOC->CRH |= (0x2 << 20);
    GPIOC->BSRR = GPIO_BSRR_BS13; // Выкл

    UART_String("Hybrid Turret Control System: Online\r\n");

    while (1) {
        if (rx_flag) {
            int target_x = 0;
            int target_y = 0;

            // Парсинг пакета от Raspberry Pi (формат "X,Y")
            if (sscanf((char*)rx_buffer, "%d,%d", &target_x, &target_y) == 2) {
                
                // Визуальное подтверждение приема пакета
                GPIOC->ODR ^= GPIO_ODR_ODR13; 

                // 1. Вычисление вектора ошибки (центр кадра 320x240)
                int error_x = target_x - 320;
                int error_y = target_y - 240;

                // 2. Мертвая зона (Deadzone)
                // Критически важно для 360-серво, чтобы он не "полз" в центре
                if (error_x > -15 && error_x < 15) error_x = 0;
                if (error_y > -15 && error_y < 15) error_y = 0;
                
                // 3. Расчет интегрального звена
                integral_x += error_x;
                if(integral_x > 1000) integral_x = 1000;
                if(integral_x < -1000) integral_x = -1000;

                integral_y += error_y;
                if(integral_y > 1000) integral_y = 1000;
                if(integral_y < -1000) integral_y = -1000;

                // 4. Расчет дифференциального звена
                int derivative_x = error_x - prev_error_x;
                int derivative_y = error_y - prev_error_y;

                // 5. Вычисление управляющего сигнала (PID)
                float control_pan = (Kp * error_x) + (Ki * integral_x) + (Kd * derivative_x);
                float control_tilt = (Kp * error_y) + (Ki * integral_y) + (Kd * derivative_y);

                // 6. ПРИМЕНЕНИЕ ГИБРИДНОЙ ЛОГИКИ
                
                // Ось Y (180 град): Накапливаем позицию угла
                tilt_angle += control_tilt; 

                // Ось X (360 град): Управляем скоростью относительно нейтрали
                // Если ошибка 0, команда будет ровно x_neutral (стоп)
                float pan_speed_command = x_neutral + (control_pan * speed_k);

                // 7. Ограничители (Safeguards)
                // Лимиты для 180-серво (физические углы)
                if (tilt_angle > 180.0f) tilt_angle = 180.0f;
                if (tilt_angle < 0.0f)   tilt_angle = 0.0f;

                // Лимиты для 360-серво (безопасная скорость вращения)
                if (pan_speed_command > 110.0f) pan_speed_command = 110.0f;
                if (pan_speed_command < 70.0f)  pan_speed_command = 70.0f;

                // 8. Передача данных на ШИМ-контроллер
                Servo_Turn(1, (uint32_t)pan_speed_command); // PA0 (Скорость)
                Servo_Turn(2, (uint32_t)tilt_angle);         // PA1 (Угол)

                // Сохранение истории для следующей итерации
                prev_error_x = error_x;
                prev_error_y = error_y;
            }
            
            rx_flag = 0; // Сброс флага UART
        }
    }    
}