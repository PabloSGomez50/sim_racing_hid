#pragma once
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_panel_io_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h"
#include "demos/lv_demos.h"

#include "esp_lcd_ili9341.h"

#define ILI9341_H_RES 240
#define ILI9341_V_RES 320
#define ILI9341_SPI_HOST SPI3_HOST
#define ILI9341_SPI_CLK_GPIO 18
#define ILI9341_SPI_MOSI_GPIO 23
#define ILI9341_SPI_MISO_GPIO -1
#define ILI9341_SPI_CS_GPIO 5
#define ILI9341_SPI_DC_GPIO 21
#define ILI9341_SPI_RST_GPIO 22
#define ILI9341_BIT_PER_PIXEL 16
#define ILI9341_X_GAP 0
#define ILI9341_Y_GAP 0

#define LVGL_BUF_LEN  (ILI9341_H_RES * 20)
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2

extern lv_disp_draw_buf_t disp_buf;                                                 // contains internal graphic buffer(s) called draw buffer(s)
extern lv_disp_drv_t disp_drv;                                                      // contains callback functions
extern lv_disp_t *disp;    

bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
/* Rotate display and touch, when rotated screen in LVGL. Called when driver parameters are updated. */
void example_lvgl_port_update_callback(lv_disp_drv_t *drv);
void example_increase_lvgl_tick(void *arg);



void LVGL_Init(void);                     // Call this function to initialize the screen (must be called in the main function) !!!!!