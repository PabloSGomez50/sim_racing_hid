#include "task_functions.h"

void hardware_init(void)
{
  adc_init();
  adc_gpio_init(ADC_CH_PIN + ADC_BRAKE_CH);
  adc_gpio_init(ADC_CH_PIN + ADC_THROTTLE_CH);
  adc_select_input(ADC_BRAKE_CH);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  gpio_init(BTN_BRAKE_PIN);
  gpio_set_dir(BTN_BRAKE_PIN, GPIO_IN);

  gpio_init(BTN_THROTTLE_PIN);
  gpio_set_dir(BTN_THROTTLE_PIN, GPIO_IN);
}

int8_t sense_adc_value(uint8_t channel) {
    adc_select_input(channel);
    int8_t steer_value = (int8_t)((int16_t)(adc_read() >> 4)) - 128; // 12-bit ADC value
    if (steer_value < -127)
      return -127;
    if (steer_value > 127)
      return 127;

    return steer_value;
}

void send_hid_gamepad_report(uint32_t btn, int8_t x_axis)
{
  if (!tud_hid_ready())
    return;

  hid_gamepad_report_t report = {
    .x = x_axis,
    .y = 0,
    .z = 0,
    .rz = 0,
    .rx = 0,
    .ry = 0,
    .hat = 0,
    .buttons = btn
  };

  tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}




void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if (!tud_hid_ready())
    return;

  switch (report_id)
  {
  case REPORT_ID_KEYBOARD:
  {
    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_keyboard_key = false;

    if (btn)
    {
      uint8_t keycode[6] = {0};
      keycode[0] = HID_KEY_A;

      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
      has_keyboard_key = true;
    }
    else
    {
      // send empty key report if previously has key pressed
      if (has_keyboard_key)
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
      has_keyboard_key = false;
    }
  }
  break;

  case REPORT_ID_MOUSE:
  {
    int8_t const delta = 5;

    // no button, right + down, no scroll, no pan
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
  }
  break;

  case REPORT_ID_CONSUMER_CONTROL:
  {
    // use to avoid send multiple consecutive zero report
    static bool has_consumer_key = false;

    if (btn)
    {
      // volume down
      uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
      tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
      has_consumer_key = true;
    }
    else
    {
      // send empty key report (release key) if previously has key pressed
      uint16_t empty_key = 0;
      if (has_consumer_key)
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
      has_consumer_key = false;
    }
  }
  break;

  case REPORT_ID_GAMEPAD:
  {
    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_gamepad_key = false;

    hid_gamepad_report_t report =
        {
            .x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0, .hat = 0, .buttons = 0};

    if (btn)
    {
      report.hat = GAMEPAD_HAT_UP;
      report.buttons = GAMEPAD_BUTTON_A;
      tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

      has_gamepad_key = true;
    }
    else
    {
      report.hat = GAMEPAD_HAT_CENTERED;
      report.buttons = 0;
      if (has_gamepad_key)
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
      has_gamepad_key = false;
    }
  }
  break;

  default:
    break;
  }
}