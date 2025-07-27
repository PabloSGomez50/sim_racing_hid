#ifndef _TASK_FUNCTIONS_H_
#define _TASK_FUNCTIONS_H_

#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "usb_descriptors.h"
#include "bsp/board_api.h"
#include "tusb.h"

#define ADC_BRAKE_CH 0
#define ADC_THROTTLE_CH 1
#define ADC_CH_PIN 26

#define NUM_BUTTONS 3
#define BTN_1_PIN 7
#define BTN_2_PIN 8
#define BTN_3_PIN 14
#define DEBOUNCE_DELAY_US 50 * 1000

typedef struct {
    uint8_t gpio;
    bool pressed;
    bool debounced;
    uint32_t last_time_us;
} button_state_t;

extern volatile button_state_t btns_hid_states[NUM_BUTTONS];

void gpio_callback(uint gpio, uint32_t events);
void check_debounced_buttons(void);

int8_t sense_adc_value(uint8_t channel);

void hardware_init(void);
void send_hid_gamepad_report(uint32_t btn, int8_t x_axis);

void send_hid_report(uint8_t report_id, uint32_t btn);

#endif