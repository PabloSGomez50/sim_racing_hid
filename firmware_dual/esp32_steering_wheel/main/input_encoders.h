#ifndef INPUT_ENCODERS_H
#define INPUT_ENCODERS_H

#include <stdint.h>

#include "esp_err.h"

esp_err_t input_encoders_init(void);
void input_encoders_poll(void);
uint8_t input_encoders_count(void);
int16_t input_encoders_get_delta_and_clear(uint8_t idx);
uint16_t input_encoders_get_event_id(uint8_t idx);

#endif
