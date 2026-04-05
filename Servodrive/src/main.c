#include "servo.h"
#include "uart_driver.h"
#include "sys.core.h"
#include <stdio.h> // Необходим для sscanf

// Базовые углы обзора при старте (Центр)
int pan_angle = 90;
int tilt_angle = 90;

int main(void) {
    RCC_System_72HZ();
    UART_Init();
    Servo_Init();

    UART_String("Turret Tracking System Online.\r\n");

    while (1) {
        // Проверяем, собран ли пакет в прерывании
        if (rx_flag) {
            int target_x = 0;
            int target_y = 0;

            // Парсинг пакета "X,Y"
            if (sscanf((char*)rx_buffer, "%d,%d", &target_x, &target_y) == 2) {
                
                // Вычисление вектора ошибки (цель всегда должна быть в центре: 320x240)
                int error_x = target_x - 320;
                int error_y = target_y - 240;

                // П-регулятор с мертвой зоной (Deadzone), чтобы избежать дрожания сервоприводов
                if (error_x > 30) pan_angle -= 1;  // Цель правее центра -> поворот
                if (error_x < -30) pan_angle += 1; // Цель левее центра -> поворот
                
                if (error_y > 30) tilt_angle += 1; // Цель ниже центра -> наклон
                if (error_y < -30) tilt_angle -= 1;// Цель выше центра -> подъем

                // Жесткие механические лимиты
                if (pan_angle > 180) pan_angle = 180;
                if (pan_angle < 0) pan_angle = 0;
                if (tilt_angle > 180) tilt_angle = 180;
                if (tilt_angle < 0) tilt_angle = 0;

                // Передача новых углов на приводы
                Servo_Turn(1, pan_angle);
                Servo_Turn(2, tilt_angle);
            }
            
            // Сброс флага готовности, ждем следующий пакет
            rx_flag = 0; 
        }
    }    
}