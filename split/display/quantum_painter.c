#include <qp.h>
#include "common.h"
#include "display/computer.qff.h"
#include "display/logo.qgf.h"

#pragma message "TAP_DANCE_ENABLE " DESCRIBE_MACRO_VALUE(TAP_DANCE_ENABLE)

static painter_font_handle_t display_font;
static painter_image_handle_t logo;
static painter_device_t display;
static uint32_t display_logo_timer = 0;
static deferred_token logo_animation_token = 0;

extern char* get_current_layer_name(void); // функция должна быть определена в keymap.c
extern char os_layout[];
extern void last_matrix_activity_trigger(void);

void display_on(bool on) {
    qp_power(display, on);
    if(on) {
      last_matrix_activity_trigger();
    }
}

void init_logo_timer(void) {
  display_logo_timer = timer_read32();
  if(display_logo_timer == 0) {
    display_logo_timer++;
  }
  if (logo_animation_token != 0) {
    qp_stop_animation(logo_animation_token);
    logo_animation_token = 0;
  }
  // показать картинку
  logo_animation_token = qp_animate(display, 0, 0, logo);
}  

void keyboard_post_init_user_display(void) {
  display = qp_sh1106_make_i2c_device(128, 32, 0x3C /* или 0x78 или 0x7A */);
  display_font = qp_load_font_mem(font_quantum_computer_font);
  logo = qp_load_image_mem(gfx_logo);
  qp_init(display, QP_ROTATION_90);
  init_logo_timer();
}

void start_display_logo(void) {
  char buf[200];
  sprintf(buf, "display: %p, display_font %p, gfx_logo: %p, gfx_logo_length: %ld, logo: %p\n", 
    display, display_font, gfx_logo, gfx_logo_length, logo);
  print(buf);
  init_logo_timer();
}

void ui_task(void) {
  static uint8_t last_led_bits = 0;
  static char old_os_layout[2];
  static char old_layer_name[2];
  
  // display_logo_timer != 0, то происходит показ логотипа
  if(display_logo_timer) {
    if (timer_elapsed32(display_logo_timer) <= SHOW_LOGO_DURATION) {
      return;
    } else {
      // отключить показ логотипа:
      display_logo_timer = 0;
      qp_stop_animation(logo_animation_token);
      logo_animation_token = 0;
      qp_clear(display);
      // обнулить раскладку, светодиоды и слой, чтобы они сразу перерисовались:
      last_led_bits = 0xff;
      old_os_layout[0] = 0;
      old_layer_name[0] = 0;
    }
  }

  static uint32_t last_draw_time = 0;
  if (timer_elapsed32(last_draw_time) <= 100) { // Throttle to 10fps
    return;
  }
  last_draw_time = timer_read32();
  
  led_t led_state = host_keyboard_led_state();
  uint8_t led_bits = 
    (led_state.caps_lock ? 4 : 0) |
    (led_state.num_lock ? 2 : 0)
  ;
  bool enable_display = false;
  // Перерисовка экрана, только если изменилось состояние CAPS/NUM:
  if(last_led_bits != led_bits) {
    // qp_power(display, true);
    last_led_bits = led_bits;
  
    char leds[] = { ' ', ' ', 0 };
    if(led_bits & 4) {
      leds[0] = 'C';
    }
    if(led_bits & 2) {
      leds[1] = 'N';
    }
    qp_drawtext(display, 0, 25, display_font, leds);
    enable_display = true;
  }

  if((os_layout[0] != old_os_layout[0]) || (os_layout[1] != old_os_layout[1])) {
    // Перерисовка только если язык ввода изменился
    old_os_layout[0] = os_layout[0];
    old_os_layout[1] = os_layout[1];
    qp_drawtext(display, 0, 0, display_font, os_layout);
    enable_display = true;
  }

  char * current_layer_name = get_current_layer_name();
  if((current_layer_name[0] != old_layer_name[0]) || (current_layer_name[1] != old_layer_name[1])) {
    // Перерисовка только если слой изменился
    old_layer_name[0] = current_layer_name[0];
    old_layer_name[1] = current_layer_name[1];
    qp_drawtext(display, 0, 50, display_font, current_layer_name);
    enable_display = true;
  }
  if(enable_display) {
    display_on(true);
  }
}