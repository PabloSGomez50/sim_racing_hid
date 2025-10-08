#ifndef _TASK_FUNCTIONS_H
#define _TASK_FUNCTIONS_H

#include "pico/stdlib.h"


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

volatile button_state_t buttons[NUM_BUTTONS] = {
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
    },
    {
        .gpio = BTN_3_PIN,
        .pressed = false,
        .debounced = true,
        .last_time_us = 0
    }
};

void gpio_callback(uint gpio, uint32_t events);
void check_debounced_buttons(void);

void hardware_init(void);


#endif