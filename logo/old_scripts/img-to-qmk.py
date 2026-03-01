#!/usr/bin/python3

import sys
import os
import argparse
import textwrap
from PIL import Image, ImageOps

IMAGE_WIDTH = 128
IMAGE_HEIGHT = 32

def convert_image(image_path, rotation=0, invert=False, threshold=128):
    try:
        img = Image.open(image_path)
    except IOError:
        print(f"Ошибка: Не удалось открыть файл {image_path}")
        return


    # Изменяем размер до логических размеров экрана
    img = img.resize((IMAGE_WIDTH, IMAGE_HEIGHT), Image.Resampling.LANCZOS)
    
    # Конвертация в ч/б
    img = img.convert('L')
    img = img.point(lambda p: 255 if p > threshold else 0)
    img = img.convert('1')


    # будущий framebuffer SSD1306
    buffer = []
    for byte_number in range(IMAGE_HEIGHT * IMAGE_WIDTH // 8):
      byte = 0
      bit_mask = 1
      for bit_number in range(8):
        # найдем координаты пикселя исходного изображения, соответствующего текущему номеру байта, бита и ориентации:
        match rotation:
          case 0:
            page_number = byte_number // IMAGE_WIDTH
            x = byte_number % IMAGE_WIDTH
            y = page_number * 8 + bit_number
          case 90:
            page_number = byte_number // IMAGE_HEIGHT
            x = page_number * 8 + bit_number
            y = IMAGE_HEIGHT - byte_number % IMAGE_HEIGHT - 1
          case 180:
            page_number = byte_number // IMAGE_WIDTH
            x = IMAGE_WIDTH - byte_number % IMAGE_WIDTH - 1
            y = IMAGE_HEIGHT - (page_number * 8 + bit_number) - 1
          case 270:
            page_number = byte_number // IMAGE_HEIGHT
            x = IMAGE_WIDTH - (page_number * 8 + bit_number) - 1
            y = byte_number % IMAGE_HEIGHT
          case _:
            raise Exception("unknown rotation")
        pixel_value = img.getpixel((x,y))
        if ((pixel_value > 0) ^ invert): 
          byte |= bit_mask
        bit_mask <<= 1
      buffer.append(byte)

    # --- Вывод результата ---
    filename = os.path.splitext(os.path.basename(image_path))[0]
    var_name = f"logo_{filename.replace(' ', '_').lower()}"
    
    print(f"// Image: {image_path}")
    print(f"// Target Rotation: {rotation} degrees")
    print(f"const unsigned char PROGMEM {var_name}[] = {{")
    
    for i in range(0, len(buffer), 16):
        chunk = buffer[i:i+16]
        hex_strings = [f"0x{b:02X}" for b in chunk]
        if i % 128 == 0 and i != 0: print("")
        print("    " + ", ".join(hex_strings) + ",")
        
    print("};")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = textwrap.dedent('''
      Конвертер изображений в массив байт QMK (SSD1306) с учетом поворота.
      Заточен только под экран 128x32.
      В QMK функция oled_write_raw_P вроде как должна записывать массив байт напрямую в буфер экрана,
      однако на самом деле при помещении массива байт в буфер, происходит их поворот с учетом настроенной ориентации экрана
      (OLED_ROTATION_90, OLED_ROTATION_180, OLED_ROTATION_270). И при вертикальном расположении (OLED_ROTATION_90 или OLED_ROTATION_270)
      на экране вместо логотипа - мусор.
      Поэтому необходимо преобразовывать картинку в массив байт с учетом будущей ориентации экрана, чем и занимается эта утилитка.
      На входе ожидается изображение в любом формате, который поддерживает библиотека Python Image Library.
      Изображение должно быть с соотношением сторон 4:1.
      Изображение будет преобразовано в монохромное, приведено к разрешению экрана (128x32) и выдано в виде массива байт,
      пригодного для передачи в oled_write_raw_P. 
      '''),
      formatter_class = argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("image", help="Путь к файлу изображения")
    parser.add_argument(
      "-r", "--rotation", 
      type = int, 
      choices = [0, 90, 180, 270], 
      default = 0, 
      help="Угол поворота экрана в QMK (0, 90, 180, 270).")
    parser.add_argument(
      "-i", "--invert",
      action="store_true",
      default = False,
      help="Инвертировать цвета (черный фон, белый текст)")

    args = parser.parse_args()
    
    print("Активные параметры:")
    print(args)
    convert_image(args.image, rotation=args.rotation, invert=args.invert)