#include QMK_KEYBOARD_H
#include <raw_hid.h>
#include "display/quantum_painter.h"

extern backlight_config_t backlight_config;

enum my_keycodes {
    EK_LOGO = SAFE_RANGE // KEYCODE для показа логотипа на дисплее и выдачи в отладочную консоль всякой информации
};

enum {
    TD_SHIFT_CAPS
};

// tap_dance_action_t tap_dance_actions[] = {
//     [TD_SHIFT_CAPS] = ACTION_TAP_DANCE_DOUBLE(KC_LSFT, KC_CAPS)
// };

// Переменная, где будет храниться текущая раскладка
char os_layout[] = "??";

enum layer_number {
    _LAYER_MAIN = 0,
    _LAYER_ALTERNATIVE,
    _LAYER_CONTROL,
};

char* get_current_layer_name(void) {
    switch (get_highest_layer(layer_state)) {
        case _LAYER_MAIN: return "L1";
        case _LAYER_ALTERNATIVE: return "L2";
        case _LAYER_CONTROL: return "L3";
        default: return "L?";
    }
}

#define MO_ALT MO(_LAYER_ALTERNATIVE)
#define MO_CTRL MO(_LAYER_CONTROL)
#define TO_CTRL TO(_LAYER_CONTROL)
#define TO_MAIN TO(_LAYER_MAIN)

#define QK_BOOT QK_BOOTLOADER
#define KC_RBGI  RGUI_T(KC_RBRC)
#define KC_BSCT  RCTL_T(KC_BSLS)
#define KC_QTSH  RSFT_T(KC_QUOT)
#define KC_SLSF  RSFT_T(KC_LSCR)
#define KC_NLCT  RCTL_T(KC_LNUM)
#define KC_ETAL  RALT_T(KC_ENT)
#define TD_CPS1 TD(TD_SHIFT_CAPS)

// Раскладка Jiran, почти оригинальная
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_LAYER_MAIN] = LAYOUT_split_3x6_5(
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                      KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                      KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        TD_CPS1, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,                      KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QTSH,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        KC_LCTL, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                      KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_BSCT,
    //+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
        KC_LGUI,                            MO_ALT,  KC_SPC,  KC_LALT, KC_ETAL, KC_BSPC, MO_ALT,                             KC_RBGI
    //+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
    ),

    [_LAYER_ALTERNATIVE] = LAYOUT_split_3x6_5(
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,                     KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_EQL,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        KC_TAB,  _______, KC_HOME, TO_CTRL, KC_PGUP, TO_CTRL,                   TO_CTRL, KC_PGUP, TO_CTRL, KC_HOME, KC_INS,  KC_DEL,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        TD_CPS1, _______, KC_LEFT, KC_UP,   KC_RGHT, _______,                   _______, KC_LEFT, KC_UP,   KC_RGHT, KC_PSCR, KC_SLSF,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        KC_LCTL, _______, KC_END,  KC_DOWN, KC_PGDN, _______,                   _______, KC_PGDN, KC_DOWN, KC_END,  KC_PAUS, KC_NLCT,
    //+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
        KC_F11,                             _______, KC_SPC,  KC_LALT, KC_ETAL, KC_DEL,  _______,                            KC_F12
    //+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
    ),
// Слой для управления
    [_LAYER_CONTROL] = LAYOUT_split_3x6_5(
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        TO_MAIN, DB_TOGG, TO_MAIN, QK_BOOT, QK_RBT,  EK_LOGO,                   EK_LOGO, QK_RBT,  QK_BOOT, TO_MAIN, DB_TOGG, TO_MAIN,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN, BL_UP,   KC_VOLU,                   KC_VOLU, BL_UP,   TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN, BL_TOGG, KC_MUTE,                   KC_MUTE, BL_TOGG, TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN,
    //+--------+--------+--------+--------+--------+--------+                 +--------+--------+--------+--------+--------+--------+
        TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN, BL_DOWN, KC_VOLD,                   KC_VOLD, BL_DOWN, TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN,
    //+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
        TO_MAIN,                            TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN, TO_MAIN,                            TO_MAIN
    //+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
    )

};

void my_backlight_enable(void) {
    // Стандартные функции изменения подсветки пишут в EEPROM/FLASH, поэтому будем управлять подсветкой напрямую
    // backlight_config.enable = 1;
    // Применяем уровень яркости к железу (если выключено - 0)
    uint8_t lvl = backlight_config.enable ? backlight_config.level : BACKLIGHT_DEFAULT_LEVEL;
    backlight_set(lvl ? lvl : 1);
}

void my_backlight_disable(void) {
    // Стандартные функции изменения подсветки пишут в EEPROM/FLASH, поэтому будем управлять подсветкой напрямую
    // backlight_config.enable = 0;
    // Применяем уровень яркости к железу (если выключено - 0)
    backlight_set(0);
}

// TODO: определиться, в какой файл перенести это:
void keyboard_post_init_user(void) {
    debug_enable = true; // Если = false, всё равно выводится матрица
    debug_matrix = false; // печатать матрицу кнопок при изменении их состояния
    debug_keyboard = true; // Печатать события клавиатуры при нажатиях:
    debug_mouse = false;
    keyboard_post_init_user_display();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case EK_LOGO:
            if (!record->event.pressed) {
                if(is_keyboard_master()) {
                    dprint("I'm master half\n");
                } else {
                    dprint("I'm slave half\n");
                }
                if(is_keyboard_left()) {
                    dprint("I'm left half\n");
                } else {
                    dprint("I'm right half\n");
                }
                // при отпускании кнопки снова отобразим на дисплее логотип на определенное время
                start_display_logo();
            }
            return false; // Skip all further processing of this key
        default: return true; // Process all other keycodes normally
    }
}

// Функция приема данных от компьютера
void raw_hid_receive_user(uint8_t *data, uint8_t length) {
    // Договоримся, что если первый байт данных равен 0x42, 
    // значит это сообщение о смене раскладки.
    if (data[0] == 0x42) {
        os_layout[0] = data[1];
        os_layout[1] = data[2];
        // display_on(true);
        if((os_layout[0] == 'E' && os_layout[1] == 'N') || (os_layout[0] == 'U' && os_layout[1] == 'S')) {
            // Для английского языка выключим подсветку
            my_backlight_disable();
        } else {
            // Для остальных языков - включим
            my_backlight_enable();
        }
    }
}

void housekeeping_task_user(void) {
    // Draw the display
    ui_task();
}
