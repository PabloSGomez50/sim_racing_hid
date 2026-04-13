#ifndef INPUT_BUTTONS_H
#define INPUT_BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

esp_err_t input_buttons_init(void);
void input_buttons_poll(void);
uint8_t input_buttons_count(void);
bool input_buttons_get_state(uint8_t idx);
uint16_t input_buttons_get_event_id(uint8_t idx);

#endif
