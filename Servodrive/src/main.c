#include "servo.h"
#include "uart_driver.h"
#include "sys.core.h"

int main(void){

    RCC_System_72HZ();

    UART_Init();

    Servo_Init();

    UART_String("Начала исследования...\r\n");

    while (1)
    {
        UART_String("Введите канал (1-2): ");
        while(!rx_flag);
        uint8_t ch = rx_buffer - '0';
        rx_flag = 0;
        UART_String("Введите угол поворота (0-180): ");
        while(!rx_flag);
        uint8_t ang = rx_buffer;
        rx_flag = 0;
        Servo_Turn(ch, ang);
    }    
}