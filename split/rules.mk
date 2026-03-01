CONSOLE_ENABLE = yes
BACKLIGHT_ENABLE = yes
BACKLIGHT_DRIVER = pwm

OLED_ENABLE = no
QUANTUM_PAINTER_ENABLE = yes          # Включаем новую графику
QUANTUM_PAINTER_DRIVERS += sh1106_i2c # Указываем драйвер вашего дисплея (SH1106 по I2C)
QUANTUM_PAINTER_LVGL_INTEGRATION = no # Пока не нужна, оставляем выключенной
QUANTUM_PAINTER_FONTS = yes
SRC+=display/quantum_painter.c
SRC+=display/computer.qff.c
SRC+=display/logo.qgf.c

# BACKLIGHT_DRIVER = pwm, timer, custom, software

# Для получения сообщений о смене раскладки от хоста:
RAW_ENABLE = yes
