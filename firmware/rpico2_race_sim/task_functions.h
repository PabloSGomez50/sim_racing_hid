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

#define BTN_BRAKE_PIN 14
#define BTN_THROTTLE_PIN 15

int8_t sense_adc_value(uint8_t channel);

void hardware_init(void);
void send_hid_gamepad_report(uint32_t btn, int8_t x_axis);

void send_hid_report(uint8_t report_id, uint32_t btn);

#endif