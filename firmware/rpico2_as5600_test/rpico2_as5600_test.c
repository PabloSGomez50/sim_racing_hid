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

    while (true) {
        as5600_status_t status = get_as5600_status(I2C_PORT);
        uint16_t angle = get_as5600_angle(I2C_PORT);
        uint8_t agc = get_as5600_agc(I2C_PORT);
        printf("Magnet Status: %d %d %d, Angle: %d, AGC: %d\n", status.mh, status.ml, status.md, angle, agc);

        sleep_ms(50);
    }
}
