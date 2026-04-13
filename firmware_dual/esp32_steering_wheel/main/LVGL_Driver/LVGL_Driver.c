#include "LVGL_Driver.h"

static const char *TAG_LVGL = "WS_LVGL";
static const char *TAG = "LVGL_Driver";
#define LVGL_SELECTED_ROTATION LV_DISP_ROT_90
static lv_color_t buf1[ LVGL_BUF_LEN ];
static lv_color_t buf2[ LVGL_BUF_LEN];
// static lv_color_t* buf1 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN , MALLOC_CAP_SPIRAM);
// static lv_color_t* buf2 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN , MALLOC_CAP_SPIRAM);
    

lv_disp_draw_buf_t disp_buf;                                                 // contains internal graphic buffer(s) called draw buffer(s)
lv_disp_drv_t disp_drv;                                                      // contains callback functions

esp_lcd_panel_handle_t panel_handle = NULL;

static void lcd_init(void) {
    ESP_LOGI(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = ILI9341_PANEL_BUS_SPI_CONFIG(
        ILI9341_SPI_CLK_GPIO,
        ILI9341_SPI_MOSI_GPIO,
        ILI9341_H_RES * 80 * ILI9341_BIT_PER_PIXEL / 8);
    ESP_ERROR_CHECK(spi_bus_initialize(ILI9341_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));


    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = ILI9341_PANEL_IO_SPI_CONFIG(
        ILI9341_SPI_CS_GPIO, ILI9341_SPI_DC_GPIO,
            example_notify_lvgl_flush_ready, NULL);
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(
        (esp_lcd_spi_bus_handle_t)ILI9341_SPI_HOST,
        &io_config,
        &io_handle
    ));

    ESP_LOGI(TAG, "Install ili9341 panel driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = ILI9341_SPI_RST_GPIO, // Shared with Touch reset
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
#elif ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(6, 0, 0)
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
#else
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
#endif
        .bits_per_pixel = ILI9341_BIT_PER_PIXEL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, true));
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    ESP_ERROR_CHECK(esp_lcd_panel_disp_off(panel_handle, false));
#else
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
#endif

}

void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    (void)panel_io;
    (void)edata;
    (void)user_ctx;
    lv_disp_flush_ready(&disp_drv);
    return false;
}

void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

/* Rotate display and touch, when rotated screen in LVGL. Called when driver parameters are updated. */
void example_lvgl_port_update_callback(lv_disp_drv_t *drv)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

    switch (drv->rotated) {
    case LV_DISP_ROT_NONE:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, true, false);
        esp_lcd_panel_set_gap(panel_handle, ILI9341_X_GAP, ILI9341_Y_GAP);
        break;
    case LV_DISP_ROT_90:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, true);
        esp_lcd_panel_set_gap(panel_handle, ILI9341_Y_GAP, ILI9341_X_GAP);
        break;
    case LV_DISP_ROT_180:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, true);
        esp_lcd_panel_set_gap(panel_handle, ILI9341_X_GAP, ILI9341_Y_GAP);
        break;
    case LV_DISP_ROT_270:
        // Rotate LCD display
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, false, false);
        esp_lcd_panel_set_gap(panel_handle, ILI9341_Y_GAP, ILI9341_X_GAP);
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
lv_disp_t *disp;
void LVGL_Init(void)
{
    ESP_LOGI(TAG_LVGL, "Initialize LVGL library");
    lcd_init();
    lv_init();
    
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LVGL_BUF_LEN );                              // initialize LVGL draw buffers

    ESP_LOGI(TAG_LVGL, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);                                                                        // Create a new screen object and initialize the associated device
    disp_drv.hor_res = ILI9341_H_RES;             
    disp_drv.ver_res = ILI9341_V_RES;                                                     // Horizontal pixel count
    disp_drv.rotated = LV_DISP_ROT_NONE;
    disp_drv.flush_cb = example_lvgl_flush_cb;                                                          // Function : copy a buffer's content to a specific area of the display
    disp_drv.drv_update_cb = example_lvgl_port_update_callback;                                         // Function : Rotate display and touch, when rotated screen in LVGL. Called when driver parameters are updated. 
    disp_drv.draw_buf = &disp_buf;                                                                      // LVGL will use this buffer(s) to draw the screens contents
    disp_drv.user_data = panel_handle;                
    ESP_LOGI(TAG_LVGL,"Register display indev to LVGL");                                                  // Custom display driver user data
    disp = lv_disp_drv_register(&disp_drv);                                                  // Create screen objects
    lv_disp_set_rotation(disp, LVGL_SELECTED_ROTATION);
    /********************* LVGL *********************/
    ESP_LOGI(TAG_LVGL, "Install LVGL tick timer");
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

}
