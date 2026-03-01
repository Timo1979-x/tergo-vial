#!/usr/bin/python3
# -*- coding: UTF-8 -*-

from PIL import Image
import random
import os

# --- НАСТРОЙКИ ---
INPUT_FILE = './logo.png'  # Исходное изображение 320x1280
OUTPUT_DIR = 'logo_frames'            # Папка для готовых кадров
FRAMES_COUNT = 8                # Количество кадров анимации

# Настройки GIF
GIF_NAME = 'logo.gif' # Имя итогового GIF-файла
FRAME_DURATION = 200              # Длительность одного кадра в миллисекундах (100мс = 10 FPS)
END_PAUSE = 1400                  # Пауза на последнем (полностью готовом) кадре в мс
# -----------------

def generate_animation():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # 1. Открываем, уменьшаем без сглаживания и жестко переводим в 1-bit
    img = Image.open(INPUT_FILE).convert('L')
    img_small = img.resize((32, 128), Image.Resampling.NEAREST)
    img_1bit = img_small.convert('1', dither=Image.Dither.NONE)
    
    width, height = img_1bit.size
    active_pixels = []
    print(f"width {width}, height {height}")

    # 2. Собираем координаты черных пикселей (0)
    for y in range(height):
        for x in range(width):
            if img_1bit.getpixel((x, y)) > 0:
                active_pixels.append((x, y))

    random.shuffle(active_pixels)
    pixels_per_frame = len(active_pixels) // FRAMES_COUNT

    # 3. Создаем первый кадр и список для GIF
    current_frame = Image.new('1', (width, height), 0)
    current_frame.save(f'{OUTPUT_DIR}/frame_00.png')
    
    gif_frames = [current_frame.copy()]  # Добавляем ПЕРВЫЙ кадр в память
    gif_durations = [FRAME_DURATION]     # Список длительностей каждого кадра

    # 4. Постепенно добавляем пиксели
    for i in range(1, FRAMES_COUNT + 1):
        start_idx = (i - 1) * pixels_per_frame
        end_idx = i * pixels_per_frame if i < FRAMES_COUNT else len(active_pixels)

        for px in active_pixels[start_idx:end_idx]:
            current_frame.putpixel(px, 255)

        # Сохраняем как PNG
        current_frame.save(f'{OUTPUT_DIR}/frame_{i:02d}.png')
        
        # Делаем "снимок" состояния для GIF и сохраняем в список
        gif_frames.append(current_frame.copy())
        
        # Если это последний кадр - ставим ему большую длительность
        if i == FRAMES_COUNT:
            gif_durations.append(END_PAUSE)
        else:
            gif_durations.append(FRAME_DURATION)

        print(f'Сохранен frame_{i:02d}.png ({end_idx}/{len(active_pixels)} пикселей)')

    # 5. Собираем и сохраняем GIF-анимацию
    print(f"Сборка {GIF_NAME}...")
    gif_frames[0].save(
        GIF_NAME,
        save_all=True,
        append_images=gif_frames[1:], # Прикрепляем все кадры кроме первого
        duration=gif_durations,       # Задаем время для каждого кадра индивидуально
        loop=0                        # 0 означает бесконечный повтор
    )
    print("Done! Now run:\nrm -rf converted-image && mkdir -p converted-image && qmk painter-convert-graphics -f mono2 -o ./converted-image -i logo.gif")

if __name__ == '__main__':
    generate_animation()