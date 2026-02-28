#pragma once
// Замените значения ниже на те, что выдал скрипт python!
#define VIAL_KEYBOARD_UID {0x06, 0xc1, 0x4e, 0xa8, 0x4b, 0x13, 0xfc, 0x5c} 

// Комбинация для разблокировки (координаты матрицы: Строка, Колонка). 
// В данном примере это [0,0] (ESC) и [0,1] (Кнопка 1). Их нужно будет зажать одновременно.
#define VIAL_UNLOCK_COMBO_ROWS { 0, 0 }
#define VIAL_UNLOCK_COMBO_COLS { 0, 1 }

#define VIAL_TAP_DANCE_ENTRIES 8
#define VIAL_COMBO_ENTRIES 8

#define DYNAMIC_KEYMAP_EEPROM_MAX_ADDR 4095
