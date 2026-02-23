#include "servo.h"
#include "uart_driver.h"
#include "sys.core.h"

/**
 * @brief Точка входа в программу.
 * Реализует последовательный опрос UART для управления осями.
 */
void main(void) {
    /* Инициализация систем */
    RCC_System_72HZ();
    UART_Init();
    Servo_Init();

    UART_String("System Online. Protocol Ready...\r\n");

    while (1) {
        // Ожидание выбора канала
        UART_String("Select Channel (1-2): ");
        while(!rx_flag);
        uint8_t ch = rx_buffer - '0'; // Примечание: ожидается бинарное значение или добавить '-0' для ASCII
        rx_flag = 0;

        // Ожидание угла поворота
        UART_String("Enter Angle (0-180): ");
        while(!rx_flag);
        uint8_t ang = rx_buffer;
        rx_flag = 0;

        Servo_Turn(ch, ang);
        UART_String("\r\nCommand Executed.\r\n");
    }    
}