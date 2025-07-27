#ifndef _TASK_FUNCTIONS_H
#define _TASK_FUNCTIONS_H

#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "usb_descriptors.h"
#include "bsp/board_api.h"
#include "tusb.h"

#define ADC_BRAKE_CH 0
#define ADC_THROTTLE_CH 1
#define ADC_CH_PIN 26

#define NUM_BUTTONS 2
#define BTN_1_PIN 14
#define BTN_2_PIN 15
#define DEBOUNCE_DELAY_US 50 * 1000

typedef struct {
    uint8_t gpio;
    bool pressed;
    bool debounced;
    uint32_t last_time_us;
} button_state_t;

static button_state_t buttons[NUM_BUTTONS] = {
    {
        .gpio = BTN_1_PIN,
        .pressed = false,
        .debounced = true,
        .last_time_us = 0
    },
    {
        .gpio = BTN_2_PIN,
        .pressed = false,
        .debounced = true,
        .last_time_us = 0
    }
};

void gpio_callback(uint gpio, uint32_t events);
void check_debounced_buttons(button_state_t *buttons, uint8_t num_buttons);

int8_t sense_adc_value(uint8_t channel);

void hardware_init(void);
void send_hid_gamepad_report(uint32_t btn, int8_t x_axis);

void send_hid_report(uint8_t report_id, uint32_t btn);



#endif