#include "as5600.h"

void init_as5600_dir() {
    gpio_init(AS5600_DIR_PIN);
    gpio_set_dir(AS5600_DIR_PIN, GPIO_OUT);
    gpio_put(AS5600_DIR_PIN, 0);
}

void set_as5600_dir(uint8_t dir) {
    gpio_put(AS5600_DIR_PIN, dir);
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

int8_t process_as5600_angle(uint16_t angle, uint16_t ref_angle) {
    int16_t diff = (int16_t)angle - (int16_t)ref_angle;
    // Handle wrap-around (0-4095)
    if (diff > 2048) diff -= 4096;
    if (diff < -2048) diff += 4096;
    // Scale to -127 to 127
    int16_t out_angle = (diff * 127) / 2048;
    if (out_angle > 127) out_angle = 127;
    if (out_angle < -127) out_angle = -127;
    return (int8_t) out_angle;
}