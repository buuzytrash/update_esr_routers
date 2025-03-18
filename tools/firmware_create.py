# Создание файла firmware.bin
with open("firmware.bin", "wb") as f:
    # Заголовок: версия прошивки (6 байт)
    version = b"2.0.0"  # Версия прошивки
    f.write(version.ljust(10, b'\x00'))  # Дополняем до 10 байт

    # Произвольные данные (имитация кода прошивки)
    firmware_data = bytes([i % 256 for i in range(1024)])  # 1024 байта данных
    f.write(firmware_data)

print("Файл firmware.bin создан.")