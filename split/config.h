#pragma once

// прошивка двойным нажатием на сброс https://docs.qmk.fm/platformdev_rp2040#double-tap
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP25
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U

#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_TX_PIN GP0
#define SERIAL_USART_RX_PIN GP1

//#define SERIAL_DEBUG

// #define WS2812_PIO_USE_PIO1

/*
 * Feature disable options
 *  These options are also useful to firmware size reduction.
 */

// disable debug print:
#define NO_DEBUG
// force enable debug output:
// #undef NO_DEBUG

// disable print:
//#define NO_PRINT
// force enable print:
#undef NO_PRINT

/* disable action features */
//#define NO_ACTION_LAYER
//#define NO_ACTION_TAPPING
//#define NO_ACTION_ONESHOT

// значения по умолчанию для oled:
// #define I2C_DRIVER I2CD1
// #define I2C1_SDA_PIN GP2
// #define I2C1_SCL_PIN GP3

#define I2C_DRIVER I2CD0
#define I2C1_SDA_PIN GP16
#define I2C1_SCL_PIN GP17

// Выкидывать в отладочную QMK console сообщения о частоте сканирования матрицы кнопок:
#define DEBUG_MATRIX_SCAN_RATE
/**
 * по состоянию этого пина прошивка определяет, работает она на левой части клавы или на правой
 * VCC - левая часть, GND - правая
 */
#define SPLIT_HAND_PIN GP22

// /** ПОДСВЕТКА */
// Для того, чтобы скомпилировалась поддержка обычной подсветки с использованием аппаратного PWM:
#define BACKLIGHT_PWM_DRIVER PWMD7
// pwm channel зависит от того, какой вывод используется для PWM
#define BACKLIGHT_PWM_CHANNEL RP2040_PWM_CHANNEL_
// это мне посоветовал gemimi, но вроде бы нигде в кодовой базе нет такой константы:
#define SPLIT_BACKLIGHT_ENABLE

// // Указываем пин, к которому подключен затвор транзистора
#define BACKLIGHT_PIN GP15

// // Количество уровней яркости (не считая состояния "выключено")
#define BACKLIGHT_LEVELS 7

// // 0 означает, что подсветка включается при подаче логического 0 (Low) на пин.
// // 1 - для N-канальных транзисторов, 0 - для P-канальных.
#define BACKLIGHT_ON_STATE 1

// Breathing работает корявенько - виден не плавный переход между уровнями яркости. Посему отключаем
#undef BACKLIGHT_BREATHING
// #define BACKLIGHT_BREATHING 1
// #define BREATHING_PERIOD 4

// // Уровень подсветки по умолчанию (после очистки EEPROM):
#define BACKLIGHT_DEFAULT_LEVEL 1

// Опционально: настройка лимита яркости (если светодиоды греются)
// 255 = 100% мощности.
#define BACKLIGHT_LIMIT_VAL 255

// Длительность показа лого в миллисекундах
#define SHOW_LOGO_DURATION 4000
#undef QUANTUM_PAINTER_DISPLAY_TIMEOUT
#define QUANTUM_PAINTER_DISPLAY_TIMEOUT 10000
