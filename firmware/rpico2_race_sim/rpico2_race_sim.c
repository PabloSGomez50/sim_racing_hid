/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "task_functions.h"
#include "config.h"
#include "as5600.h"
#include "picoRGB.h"
#include "lvgl.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);
void config_reference(void);
void wait_btn(bool btn, uint8_t btn_num);


gamepad_vars_t gamepad_vars = {
  .ref_angle = 0,
  .min_brk_adc = 0,
  .max_brk_adc = 4096,
  .min_throttle_adc = 0,
  .max_throttle_adc = 4096
};

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  hardware_init();
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);
  
  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400*1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  init_as5600_dir(AS5600_DIR_PIN);

  strip_init(16, 32);
  strip_set_brightness(10);

  if (board_init_after_tusb)
  {
    board_init_after_tusb();
  }

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();
    check_debounced_buttons();
    if (btns_hid_states[0].pressed && btns_hid_states[1].pressed)
      config_reference();
    hid_task();
    sleep_ms(5);
  }
}

void wait_btn(bool btn_state, uint8_t btn_num) {
  while(btns_hid_states[btn_num].pressed != btn_state) {
    check_debounced_buttons();
    sleep_ms(10);
  }
}

// void wait_for_config()

void config_reference(void) {
  wait_btn(false, 0);
  strip_fill_solid(0, 0, 255);
  wait_btn(true, 0);
  strip_fill_solid(0, 255, 0);
  gamepad_vars.ref_angle = get_as5600_angle(I2C_PORT);
  
  wait_btn(false, 0);
  strip_fill_solid(0, 0, 255);
  wait_btn(true, 0);
  strip_fill_solid(0, 255, 0);
  gamepad_vars.min_brk_adc = read_adc_raw(ADC_BRAKE_CH);
  
  wait_btn(false, 0);
  strip_fill_solid(0, 0, 255);
  wait_btn(true, 0);
  strip_fill_solid(0, 255, 0);
  gamepad_vars.max_brk_adc = read_adc_raw(ADC_BRAKE_CH);

  wait_btn(false, 0);
  strip_fill_solid(0, 0, 255);
  wait_btn(true, 0);
  strip_fill_solid(0, 255, 0);
  gamepad_vars.min_throttle_adc = read_adc_raw(ADC_THROTTLE_CH);
  
  wait_btn(false, 0);
  strip_fill_solid(0, 0, 255);
  wait_btn(true, 0);
  strip_fill_solid(0, 255, 0);
  gamepad_vars.max_throttle_adc = read_adc_raw(ADC_THROTTLE_CH);
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+
// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  strip_fill_solid(255, 255, 255);
  uint32_t btn = 0;
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    if (btns_hid_states[i].pressed)
      btn |= (1 << i);
  }
  
  as5600_status_t status = get_as5600_status(I2C_PORT);
  int8_t angle = process_as5600_angle(get_as5600_angle(I2C_PORT), gamepad_vars.ref_angle);
  hid_gamepad_report_t r = {
      .buttons = btn,
      .x = angle,
      .y = read_adc_raw(ADC_THROTTLE_CH),
      .z = read_adc_raw(ADC_BRAKE_CH),
      .rx = 0,
      .ry = 0,
      .rz = 0,
      .hat = 0
  };
  send_hid_gamepad_report(r);

  // Wake up host if we are in suspend mode
  // and REMOTE_WAKEUP feature is enabled by host
  if (tud_suspended())
    tud_remote_wakeup();
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len)
{
  (void)instance;
  (void)len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if (bufsize < 1)
        return;

      uint8_t const kbd_leds = buffer[0];
      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }
      else
      {
        // Caplocks Off: back to normal blink
        blink_interval_ms = BLINK_MOUNTED;
        board_led_write(false);
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  uint32_t ticks_millis = board_millis();
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms)
    return;

  // Blink every interval ms
  if (board_millis() - ticks_millis < blink_interval_ms)
    return; // not enough time
  ticks_millis = board_millis();

  gpio_put(PICO_DEFAULT_LED_PIN, led_state);
  led_state ^= 1; // toggle
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+
// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}