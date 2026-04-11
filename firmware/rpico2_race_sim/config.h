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

#endif // __CONFIG_H__