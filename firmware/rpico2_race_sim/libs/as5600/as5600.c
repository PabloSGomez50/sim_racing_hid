#include "as5600.h"

void init_as5600_dir(uint8_t dir_pin) {
    gpio_init(dir_pin);
    gpio_set_dir(dir_pin, GPIO_OUT);
    gpio_put(dir_pin, 0);
}

as5600_status_t get_as5600_status(i2c_inst_t *i2c) {
    as5600_status_t status;
    uint8_t buf;
    i2c_write_blocking(i2c, AS5600_ADDRESS, (uint8_t[]){AS5600_STATUS_REG}, 1, true);
    i2c_read_blocking(i2c, AS5600_ADDRESS, &buf, 1, false);
    buf = buf >> 3;
    status.mh  = buf & 0b001;
    status.ml = (buf & 0b010) >> 1;
    status.md = (buf & 0b100) >> 2;
    status.valid = (status.md & !status.ml & !status.mh) ? 1 : 0;

    return status;
}

uint16_t get_as5600_angle(i2c_inst_t *i2c) {
    uint8_t buffer[2];
    i2c_write_blocking(i2c, AS5600_ADDRESS, (uint8_t[]){AS5600_ANGLE_REG_HIGH}, 1, true);
    i2c_read_blocking(i2c, AS5600_ADDRESS, buffer, 2, false);
    uint16_t angle = ((buffer[0] & 0x0F) << 8 | buffer[1]);
    return angle;
}
    
uint8_t get_as5600_agc(i2c_inst_t *i2c) {
    uint8_t agc;
    i2c_write_blocking(i2c, AS5600_ADDRESS, (uint8_t[]){AS5600_AGC_REG}, 1, true);
    i2c_read_blocking(i2c, AS5600_ADDRESS, &agc, 1, false);
    return agc;
}

