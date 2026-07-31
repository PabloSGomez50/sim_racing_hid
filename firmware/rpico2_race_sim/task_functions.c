#include "task_functions.h"

volatile button_state_t btns_hid_states[NUM_BUTTONS] = {
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

void hardware_init(void)
{
  adc_init();
  adc_gpio_init(ADC_CH_PIN + ADC_BRAKE_CH);
  adc_gpio_init(ADC_CH_PIN + ADC_THROTTLE_CH);
  adc_select_input(ADC_BRAKE_CH);

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  gpio_set_irq_enabled_with_callback(btns_hid_states[0].gpio, GPIO_IRQ_EDGE_FALL, false, &gpio_callback);
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    gpio_init(btns_hid_states[i].gpio);
    gpio_set_dir(btns_hid_states[i].gpio, GPIO_IN);
    gpio_pull_up(btns_hid_states[i].gpio);
    gpio_set_irq_enabled(btns_hid_states[i].gpio, GPIO_IRQ_EDGE_FALL, true);
  }
}

void gpio_callback(uint gpio, uint32_t events)
{
  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    if (btns_hid_states[i].gpio == gpio && btns_hid_states[i].debounced)
    {
      bool state = (events & GPIO_IRQ_EDGE_FALL) != 0;
      btns_hid_states[i].pressed = state;
      btns_hid_states[i].last_time_us = time_us_32();
      btns_hid_states[i].debounced = false;
      gpio_set_irq_enabled(gpio, events, false); // Bloquear hasta confirmar
      break;
    }
  }
}

void check_debounced_buttons(void)
{
  uint32_t now = time_us_32();

  for (int i = 0; i < NUM_BUTTONS; i++)
  {
    if (!btns_hid_states[i].debounced && (now - btns_hid_states[i].last_time_us > DEBOUNCE_DELAY_US))
    {
      if (gpio_get(btns_hid_states[i].gpio))
      {
        gpio_set_irq_enabled(btns_hid_states[i].gpio, GPIO_IRQ_EDGE_FALL, true);
        btns_hid_states[i].pressed = false;
      }
      else
      {
        gpio_set_irq_enabled(btns_hid_states[i].gpio, GPIO_IRQ_EDGE_RISE, true);
      }
      btns_hid_states[i].debounced = true;
    }
  }
}

int8_t read_adc_value(uint8_t channel)
{
  adc_select_input(channel);
  int16_t axis_value = (int16_t)(adc_read() >> 4) - 128; // 12-bit ADC value
  if (axis_value < -127)
    return -127;
  if (axis_value > 127)
    return 127;

  return (int8_t) axis_value;
}

uint16_t read_adc_raw(uint8_t channel) {
    adc_select_input(channel);
    return adc_read(); // 12-bit ADC value (0-4095)
}


int8_t range_8bit_signed(uint16_t value, uint16_t ref_value) {
    int16_t diff = (int16_t)value - (int16_t)ref_value;
    // Handle wrap-around (0-4095)
    if (diff > 2048) diff -= 4096;
    if (diff < -2048) diff += 4096;
    // Scale to -127 to 127
    int16_t out_value = diff / (1 << 5);
    if (out_value > 127) out_value = 127;
    if (out_value < -127) out_value = -127;
    return (int8_t) out_value;
}

void send_hid_gamepad_report(hid_gamepad_report_t report)
{
  if (!tud_hid_ready())
    return;

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