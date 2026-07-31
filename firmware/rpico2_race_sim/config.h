#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "hardware/i2c.h"

// I2C Configuration
#define I2C_PORT    i2c0
#define I2C_SDA     12
#define I2C_SCL     13

#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 2
#endif

#define AS5600_DIR_PIN  14


/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum
{
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};


typedef struct {
  uint16_t ref_angle;
  uint16_t min_brk_adc;
  uint16_t max_brk_adc;
  uint16_t min_throttle_adc;
  uint16_t max_throttle_adc;
} gamepad_vars_t;

#endif // __CONFIG_H__