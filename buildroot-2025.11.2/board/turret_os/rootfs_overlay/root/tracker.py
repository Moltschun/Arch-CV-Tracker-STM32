import cv2
import serial
import time
import numpy as np

print("Инициализация подсистем...")

kalman = cv2.KalmanFilter(4,2)

kalman.measurementMatrix = np.array([[1, 0, 0, 0],
                                     [0, 1, 0, 0]], np.float32)

kalman.transitionMatrix = np.array([[1, 0, 1, 0],
                                     [0, 1, 0, 1],
                                     [0, 0, 1, 0],
                                     [0, 0, 0, 1]], np.float32)

kalman.processNoiseCov = np.eye(4, dtype=np.float32) * 0.03
kalman.measurementNoiseCov = np.eye(2, dtype=np.float32) * 1.0

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

# Инициализация телеметрии

log_interval = 60
last_log_time = time.time()
frame_count = 0
lost_frames = 0
start_time = time.time()

LOG_PATH = "/tmp/turret_stats.log"

try:
    while True:
        ret, frame = cap.read()
        frame_count += 1
        if not ret:
            lost_frames += 1
            print("Сбой получения кадра.")
            continue

        height, width, _ = frame.shape
	
	# 1. Переводим кадр из BGR в цветовое пространство HSV
        hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        # 2. Настраиваем фильтры для КРАСНОГО цвета.
        # Красный "разорван" на краях шкалы H (0-10 и 170-180)
        lower_red_1 = np.array([0, 120, 70])
        upper_red_1 = np.array([10, 255, 255])
        
        lower_red_2 = np.array([170, 120, 70])
        upper_red_2 = np.array([180, 255, 255])

        # 3. Создаем две отдельные маски
        mask1 = cv2.inRange(hsv_frame, lower_red_1, upper_red_1)
        mask2 = cv2.inRange(hsv_frame, lower_red_2, upper_red_2)
        
        # Склеиваем маски в одну (оставляем пиксели, которые попали либо в первую, либо во вторую)
        mask = cv2.bitwise_or(mask1, mask2)

        # Убираем цифровой шум (морфологические трансформации)
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)

        # 4. Ищем контуры КРАСНОГО объекта
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        # Базовая позиция: смотрим прямо, если цель не найдена
        aim_x = width // 2
        aim_y = height // 2
        
        # === 1. ФАЗА ПРОГНОЗА ===
        prediction = kalman.predict()
        pred_x, pred_y = prediction[0][0], prediction[1][0]
        pred_vx, pred_vy = prediction[2][0], prediction[3][0]

        # === 2. ОБРАБОТКА И КОРРЕКЦИЯ ===
        if len(contours) > 0:
            largest_contour = max(contours, key=cv2.contourArea)
            
            if cv2.contourArea(largest_contour) > 500:
                M = cv2.moments(largest_contour)
                if M["m00"] > 0:
                    current_x = int(M["m10"] / M["m00"])
                    current_y = int(M["m01"] / M["m00"])

                    # Передаем реальные координаты фильтру
                    measurement = np.array([[np.float32(current_x)], [np.float32(current_y)]])
                    kalman.correct(measurement)

                    # === 3. ВЫЧИСЛЕНИЕ УПРЕЖДЕНИЯ ===
                    multiplier_frames = 3
                    
                    aim_x = int(current_x + (pred_vx * multiplier_frames))
                    aim_y = int(current_y + (pred_vy * multiplier_frames))

                    # Защита от выхода за границы кадра
                    aim_x = max(0, min(width, aim_x))
                    aim_y = max(0, min(height, aim_y))

        # === 4. ПЕРЕДАЧА ТЕЛЕМЕТРИИ ===
        # Отправляем ПРЕДСКАЗАННЫЕ координаты на STM32
        packet = f"{aim_x},{aim_y}\r\n"
        uart.write(packet.encode('utf-8'))

        print(f"Цель: X={aim_x} Y={aim_y} | Скорость: Vx={pred_vx:.1f} Vy={pred_vy:.1f}")

        time.sleep(0.1)

        # === МОДУЛЬ ТАКТИЧЕСКОЙ ТЕЛЕМЕТРИИ ===
        current_time = time.time()
        if current_time - last_log_time >= log_interval:
            # 1. Расчет FPS (Кадры в секунду)
            actual_fps = frame_count / (current_time - last_log_time)
            
            # 2. Получение температуры CPU (специфично для Raspberry Pi)
            try:
                with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
                    temp_raw = int(f.read())
                    cpu_temp = temp_raw / 1000.0 # Преобразование в градусы Цельсия
            except:
                cpu_temp = 0.0

            # 3. Проверка статуса UART
            uart_status = "ONLINE" if uart.is_open else "OFFLINE"

            # 4. Формирование записи
            timestamp = time.strftime("%H:%M:%S")
            log_entry = (f"[{timestamp}] FPS: {actual_fps:.2f} | "
                         f"CPU Temp: {cpu_temp:.1f}C | "
                         f"Lost Frames: {lost_frames} | "
                         f"UART: {uart_status}\n")

            # Запись в RAM-диск (режим 'a' - дозапись)
            try:
                with open(LOG_PATH, "a") as log_file:
                    log_file.write(log_entry)
            except Exception as e:
                print(f"Ошибка записи лога: {e}")

            # Сброс счетчиков для следующего интервала
            last_log_time = current_time
            frame_count = 0
            # Мы не сбрасываем lost_frames, чтобы видеть общее количество сбоев с момента старта
	
except KeyboardInterrupt:
    print("\nПолучен приказ на остановку. Сворачиваю системы.")
finally:
    cap.release()
    uart.close()
    print("Связь разорвана.")
