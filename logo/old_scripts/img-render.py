#!/usr/bin/python3

import sys
import os
import re
from PIL import Image
import textwrap

# Таблица символов Unicode Quadrants (2x2 пикселя)
# Индекс формируется битовой маской: (BR << 3) | (BL << 2) | (TR << 1) | (TL)
# Где: TL=TopLeft, TR=TopRight, BL=BottomLeft, BR=BottomRight
# 0 = пиксель выключен, 1 = пиксель включен
QUADRANTS = [
    " ", "▘", "▝", "▀",
    "▖", "▌", "▞", "▛",
    "▗", "▚", "▐", "▜",
    "▄", "▙", "▟", "█"
]

WIDTH = 128
HEIGHT = 32

def parse_c_array(text):
    """Парсит C-массив и преобразует структуру SSD1306 в матрицу пикселей."""
    # Ищем все вхождения hex-чисел (например, 0xFF, 0x00)
    hex_values = re.findall(r'0x[0-9A-Fa-f]{2}', text)
    
    if not hex_values:
        raise ValueError("Не найдено hex-данных (0x..) в файле.")

    bytes_list = [int(val, 16) for val in hex_values]
    
    expected_bytes = (WIDTH * HEIGHT) // 8
    if len(bytes_list) != expected_bytes:
        print(f"Предупреждение: Ожидалось {expected_bytes} байт, найдено {len(bytes_list)}.")
        # Если данных больше, обрезаем, если меньше - дополняем нулями
        if len(bytes_list) > expected_bytes:
            bytes_list = bytes_list[:expected_bytes]
        else:
            bytes_list += [0] * (expected_bytes - len(bytes_list))

    # Создаем пустую матрицу
    pixels = [[0 for _ in range(WIDTH)] for _ in range(HEIGHT)]

    # Декодируем формат SSD1306 (Vertical Page Addressing)
    # Экран разбит на 4 страницы по 8 пикселей высотой.
    # Байты идут слева направо для каждой страницы.
    for i, byte in enumerate(bytes_list):
        page = i // WIDTH       # Номер страницы (0-3)
        col = i % WIDTH         # Колонка (0-127)
        
        for bit in range(8):
            # Если бит установлен, пиксель горит
            if byte & (1 << bit):
                y = (page * 8) + bit
                if y < HEIGHT:
                    pixels[y][col] = 1
    return pixels

def process_image(image_path):
    """Загружает изображение, ресайзит и преобразует в матрицу."""
    img = Image.open(image_path)
    img = img.resize((WIDTH, HEIGHT), Image.Resampling.NEAREST)
    img = img.convert('1') # Бинаризация
    
    pixels = [[0 for _ in range(WIDTH)] for _ in range(HEIGHT)]
    
    for y in range(HEIGHT):
        for x in range(WIDTH):
            # В PIL 255 (белый) обычно фон, 0 (черный) текст.
            # Но для OLED: 1 = свет, 0 = тьма.
            # Предположим, что входное изображение черно-белое, где белое = свет.
            p = img.getpixel((x, y))
            pixels[y][x] = 1 if p > 128 else 0
            
    return pixels

def print_quadrants(pixels):
    """Выводит матрицу пикселей в консоль с помощью символов Quadrants."""
    print(f"Preview ({WIDTH}x{HEIGHT}):")
    print("+" + "-" * (WIDTH // 2) + "+") # Рамка сверху

    # Проходим по сетке с шагом 2x2
    for y in range(0, HEIGHT, 2):
        line = "|"
        for x in range(0, WIDTH, 2):
            # Собираем 4 пикселя
            tl = pixels[y][x]
            tr = pixels[y][x+1] if x+1 < WIDTH else 0
            bl = pixels[y+1][x] if y+1 < HEIGHT else 0
            br = pixels[y+1][x+1] if (y+1 < HEIGHT and x+1 < WIDTH) else 0
            
            # Вычисляем индекс символа
            index = (tl) | (tr << 1) | (bl << 2) | (br << 3)
            line += QUADRANTS[index]
        line += "|"
        print(line)
        
    print("+" + "-" * (WIDTH // 2) + "+") # Рамка снизу

def main():
    print(textwrap.dedent('''
    Выводит в консоль изображение в виде псевдографики. На входе bitmap или массив байтов для дисплея ssd1306 из прошивки QMK.
    Консоль должна иметь шрифт с символами Unicode Quadrants.
    '''))
    if len(sys.argv) < 2:
        print("Использование: python img-render.py <файл_изображения_или_си_код>")
        return

    filepath = sys.argv[1]

    pixels = []

    try:
        # Попытка 1: Открыть как изображение
        pixels = process_image(filepath)
        print(f"Успешно загружено изображение: {filepath}")
    except Exception:
        # Попытка 2: Открыть как текст (C array)
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            pixels = parse_c_array(content)
            print(f"Успешно загружен массив байт из: {filepath}")
        except Exception as e:
            print(f"Ошибка: Не удалось распознать файл ни как изображение, ни как C-код.\n{e}")
            return

    print_quadrants(pixels)

if __name__ == "__main__":
    main()
