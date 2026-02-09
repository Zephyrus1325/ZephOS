#ifndef PS2_H
#define PS2_H

#include <stdint.h>

#define PS2_RESET

#define PS2_NUM_LOCK_LED_ON
#define PS2_NUM_LOCK_LED_OFF
#define PS2_CAPS_LOCK_LED_ON
#define PS2_CAPS_LOCK_LED_OFF
#define PS2_SCROLL_LOCK_LED_ON
#define PS2_SCROLL_LOCK_LED_OFF


void k_setup_ps2();
uint8_t k_ps2_send(uint8_t value);



#endif //PS2_H