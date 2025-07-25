#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "config.h"
#include "display/display.h"

int main()
{
    stdio_init_all();

    // InitializeDisplay(FOREGROUND);
    gpio_init(TFT_LED);
    gpio_set_dir(TFT_LED, GPIO_OUT);
    gpio_put(TFT_LED, 0);
    
    LCD_setSPIperiph(TFT_SPI_INSTANCE);
    LCD_setPins(TFT_DC, TFT_CS, TFT_RST, TFT_SCLK, TFT_MOSI);
    LCD_initDisplay();
    LCD_setRotation(TFT_ROTATION);

    display_lvgl_init();
    gpio_put(TFT_LED, 1);
    // Commodore64();
    while(1){
        lv_timer_handler();
        sleep_ms(5);
    }
    return 0;
}