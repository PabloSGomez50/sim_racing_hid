#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "as5600.h"
// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SCL 21
#define I2C_SDA 20


int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    init_as5600_dir();
    uint16_t angle_ref = 25;
    while (true) {
        as5600_status_t status = get_as5600_status(I2C_PORT);
        int16_t raw_angle = (int16_t)get_as5600_angle(I2C_PORT);
        int16_t diff = raw_angle - angle_ref;
        // Handle wrap-around (0-4095)
        if (diff > 2048) diff -= 4096;
        if (diff < -2048) diff += 4096;
        // Scale to -127 to 127
        int16_t angle = (diff * 127) / 2048;
        if (angle > 127) angle = 127;
        if (angle < -127) angle = -127;
        uint8_t agc = get_as5600_agc(I2C_PORT);
        printf("Magnet Status: %d %d %d, Angle: %d, AGC: %d\n", status.mh, status.ml, status.md, angle, agc);

        sleep_ms(50);
    }
}
