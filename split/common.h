#pragma once

/**
  Макросы для печати значения других макросов на этапе компиляции:
  Использовать так:
  #pragma message "QUANTUM_PAINTER_DISPLAY_TIMEOUT " DESCRIBE_MACRO_VALUE(QUANTUM_PAINTER_DISPLAY_TIMEOUT)
  #pragma message "QUANTUM_PAINTER_DISPLAY_TIMEOUT " DESCRIBE_MACRO(QUANTUM_PAINTER_DISPLAY_TIMEOUT)
  #pragma message DESCRIBE_MACRO(QMK_KEYBOARD_H)
*/
#define DESCRIBE_MACRO__(x) #x
#define DESCRIBE_MACRO_VALUE(x) DESCRIBE_MACRO__(x)

#define DESCRIBE_MACRO(x) #x " ==> " DESCRIBE_MACRO__(x)
