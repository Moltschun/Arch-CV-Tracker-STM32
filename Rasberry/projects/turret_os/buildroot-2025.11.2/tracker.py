import cv2
import serial
import time

print("Инициализация подсистем...")

# Инициализация UART (Связь со спинным мозгом - STM32)
# Используем освобожденный аппаратный порт PL011
try:
    uart = serial.Serial('/dev/ttyAMA0', baudrate=115200, timeout=1)
    print("Канал связи [UART] установлен.")
except serial.SerialException:
    print("Критическая ошибка: Порт UART недоступен.")
    exit()

# Инициализация камеры (Зрительный нерв)
cap = cv2.VideoCapture(0)
time.sleep(2) # Прогрев сенсора

if not cap.isOpened():
    print("Критическая ошибка: Оптический сенсор не отвечает.")
    uart.close()
    exit()

print("Системы захвата активированы. Трансляция координат начата.")

try:
    while True:
        # Захват кадра
        ret, frame = cap.read()
        if not ret:
            print("Сбой получения кадра.")
            break

        # Временно берем центр кадра как "захваченную цель"
        # Для стандартного разрешения Raspberry камеры это обычно 320x240
        height, width, _ = frame.shape
        target_x = width // 2
        target_y = height // 2

        # Формируем пакет данных. Строгий формат: "X,Y\n"
        packet = f"{target_x},{target_y}\n"

        # Отправка данных на STM32
        uart.write(packet.encode('utf-8'))

        # Логирование для Пилота
        print(f"Отправлено на STM32: {packet.strip()}")

        # Ограничение частоты (10 кадров в секунду)
        # Это защитит STM32 от переполнения буфера на этапе отладки
        time.sleep(0.1)

except KeyboardInterrupt:
    print("\nПолучен приказ на остановку. Сворачиваю системы.")
finally:
    cap.release()
    uart.close()
    print("Связь разорвана.")
