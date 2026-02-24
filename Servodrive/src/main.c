#include "servo.h"
#include "uart_driver.h"
#include "sys.core.h"
#include <stdlib.h>

/**
 * @brief Точка входа в программу.
 * Реализует последовательный опрос UART для управления осями.
 */
void main(void) {
    /* Инициализация систем */
    RCC_System_72HZ();
    UART_Init();
    Servo_Init();

    char text [10];
    uint8_t index_text = 0;

    UART_String("System Online. Protocol Ready...\r\n");

    while (1) {
        // Ожидание выбора канала
        UART_String("Select Channel (1-2): \r\n");
        while(!rx_flag);
        uint8_t ch = rx_buffer - '0'; // Примечание: ожидается бинарное значение или добавить '-0' для ASCII
        UART_String("Entered: channel");
        rx_flag = 0;

        // Ожидание угла поворота
        UART_String("Enter Angle (0-180): \r\n");
        for (uint8_t i = 0; i < 10; i++){
            while(!rx_flag);
            if (rx_buffer == '\r' || rx_buffer == '\n'){
                break;
            }

            text[i] = rx_buffer;
            
        }
        uint8_t ang = rx_buffer;
        UART_String("Entered: ang");
        rx_flag = 0;

        Servo_Turn(ch, ang);
        UART_String("\r\nCommand Executed.\r\n");
    }    
}