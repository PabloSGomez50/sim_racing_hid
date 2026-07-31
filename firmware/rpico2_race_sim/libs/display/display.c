#include "display.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[TFT_HEIGHT * 10];
static lv_color_t buf2[TFT_HEIGHT * 10];

static lv_obj_t *lbl_pos;
static lv_obj_t *lbl_lap;
static lv_obj_t *lbl_lap_time;
static lv_obj_t *lbl_gear;
static lv_obj_t *lbl_speed_kph;
static lv_obj_t *lbl_tyre_temp[4];

static lv_obj_t *btn_overtake;
static lv_obj_t *btn_drs;

static lv_obj_t *bar_batt;
static lv_obj_t *lbl_batt;


// Flush de LVGL al display físico
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    LCD_WriteBitmap(
        area->x1, area->y1,
        area->x2 - area->x1 + 1,
        area->y2 - area->y1 + 1,
        (uint16_t *)color_p
    );
    lv_disp_flush_ready(disp);
}

// Inicialización de pantalla y LVGL
void display_lvgl_init(void) {
    lv_init();
    // lv_disp_draw_buf_init(&draw_buf, buf1, NULL, sizeof(buf1)/sizeof(lv_color_t));
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, sizeof(buf1)/sizeof(lv_color_t));

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = TFT_WIDTH;
    disp_drv.ver_res = TFT_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    f1_dashboard_create();
}

void f1_dashboard_create(void) {
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    // === PANEL: Car Info (Velocidad/Vuelta/Posición) ===
    lv_obj_t *box_left = lv_obj_create(scr);
    lv_obj_set_size(box_left, 110, 90);
    lv_obj_align(box_left, LV_ALIGN_TOP_LEFT, 5, 0);
    lv_obj_set_style_bg_color(box_left, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(box_left, 0, 0);
    lv_obj_clear_flag(box_left, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_left_title = lv_label_create(box_left);
    lv_label_set_text(lbl_left_title, "Info");
    lv_obj_align(lbl_left_title, LV_ALIGN_TOP_MID, 0, -10);
    lv_obj_set_style_text_color(lbl_left_title, lv_color_black(), 0);

    lv_obj_t *box_left_inner = lv_obj_create(box_left);
    lv_obj_set_size(box_left_inner, 100, 65);
    lv_obj_align(box_left_inner, LV_ALIGN_BOTTOM_MID, 0, 8);
    lv_obj_set_style_bg_color(box_left_inner, lv_color_black(), 0);
    lv_obj_set_style_border_width(box_left_inner, 0, 0);
    lv_obj_clear_flag(box_left_inner, LV_OBJ_FLAG_SCROLLABLE);
    
    lbl_pos = lv_label_create(box_left_inner);
    lv_label_set_text(lbl_pos, "P1");
    lv_obj_align(lbl_pos, LV_ALIGN_TOP_LEFT, 2, -5);
    lv_obj_set_style_text_color(lbl_pos, lv_color_white(), 0);
    
    lbl_lap = lv_label_create(box_left_inner);
    lv_label_set_text(lbl_lap, "L15/30");
    lv_obj_align(lbl_lap, LV_ALIGN_TOP_LEFT, 2, 10);
    lv_obj_set_style_text_color(lbl_lap, lv_color_white(), 0);

    lbl_lap_time = lv_label_create(box_left_inner);
    lv_label_set_text(lbl_lap_time, "01:23.456");
    lv_obj_align(lbl_lap_time, LV_ALIGN_TOP_LEFT, 2, 25);
    lv_obj_set_style_text_color(lbl_lap_time, lv_color_white(), 0);

    // === PANEL: Gear indicator===
    lv_obj_t *box_center = lv_obj_create(scr);
    lv_obj_set_size(box_center, 80, 90);
    lv_obj_align(box_center, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(box_center, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(box_center, 0, 0);
    lv_obj_clear_flag(box_center, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_center_title = lv_label_create(box_center);
    lv_label_set_text(lbl_center_title, "Gear");
    lv_obj_align(lbl_center_title, LV_ALIGN_TOP_MID, 0, -10);
    lv_obj_set_style_text_color(lbl_center_title, lv_color_black(), 0);

    lv_obj_t *box_center_inner = lv_obj_create(box_center);
    lv_obj_set_size(box_center_inner, 70, 65);
    lv_obj_align(box_center_inner, LV_ALIGN_BOTTOM_MID, 0, 8);
    lv_obj_set_style_bg_color(box_center_inner, lv_color_black(), 0);
    lv_obj_set_style_border_width(box_center_inner, 0, 0);
    lv_obj_clear_flag(box_center_inner, LV_OBJ_FLAG_SCROLLABLE);

    lbl_gear = lv_label_create(box_center_inner);
    lv_label_set_text(lbl_gear, "5");
    lv_obj_set_style_text_font(lbl_gear, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(lbl_gear, lv_color_white(), 0);
    lv_obj_align(lbl_gear, LV_ALIGN_BOTTOM_MID, 0, -10);

    lbl_speed_kph = lv_label_create(box_center_inner);
    lv_label_set_text(lbl_speed_kph, "325");
    lv_obj_set_style_text_font(lbl_speed_kph, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(lbl_speed_kph, lv_color_white(), 0);
    lv_obj_align(lbl_speed_kph, LV_ALIGN_BOTTOM_MID, 0, 10);

    // === PANEL: Tyre Temp en grilla 2x2 ===
    lv_obj_t *box_right = lv_obj_create(scr);
    lv_obj_set_size(box_right, 110, 90);
    lv_obj_align(box_right, LV_ALIGN_TOP_RIGHT, -5, 0);
    lv_obj_set_style_bg_color(box_right, lv_color_hex(0x666666), 0);
    lv_obj_set_style_border_width(box_right, 0, 0);
    lv_obj_clear_flag(box_right, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl_right_title = lv_label_create(box_right);
    lv_label_set_text(lbl_right_title, "Tyre Temp");
    lv_obj_align(lbl_right_title, LV_ALIGN_TOP_MID, 0, -10);
    lv_obj_set_style_text_color(lbl_right_title, lv_color_black(), 0);

    lv_obj_t *box_right_inner = lv_obj_create(box_right);
    lv_obj_set_size(box_right_inner, 100, 65);
    lv_obj_align(box_right_inner, LV_ALIGN_BOTTOM_MID, 0, 8);
    lv_obj_set_style_bg_color(box_right_inner, lv_color_black(), 0);
    lv_obj_set_style_border_width(box_right_inner, 0, 0);
    lv_obj_clear_flag(box_right_inner, LV_OBJ_FLAG_SCROLLABLE);

    int temps[4] = {105, 93, 77, 89};
    for (int i = 0; i < 4; i++) {
        int row = i / 2;
        int col = i % 2;
        
        lv_obj_t *temp_box = lv_obj_create(box_right_inner);
        lv_obj_set_size(temp_box, 40, 25);
        lv_obj_align(temp_box, LV_ALIGN_TOP_LEFT, col * 45 - 5, row * 30 - 7);
        lv_obj_clear_flag(temp_box, LV_OBJ_FLAG_SCROLLABLE);

        lv_color_t color;
        if (temps[i] > 100)      color = lv_color_make(255, 0, 0);
        else if (temps[i] > 80)  color = lv_color_make(0, 255, 0);
        else                     color = lv_color_make(0, 0, 255);

        lv_obj_set_style_bg_color(temp_box, color, 0);
        lv_obj_set_style_border_width(temp_box, 0, 0);

        char txt[8];
        sprintf(txt, "%d°", temps[i]);
        lbl_tyre_temp[i] = lv_label_create(temp_box);
        lv_label_set_text(lbl_tyre_temp[i], txt);
        lv_obj_center(lbl_tyre_temp[i]);
        lv_obj_set_style_text_color(lbl_tyre_temp[i], lv_color_white(), 0);
    }

    // === PANEL: Logo centrado ===
    LV_IMG_DECLARE(Logo_Williams_F1_transparent);
    lv_obj_t *img_logo = lv_img_create(scr);
    lv_img_set_src(img_logo, &Logo_Williams_F1_transparent);
    lv_obj_align(img_logo, LV_ALIGN_CENTER, 0, 5);

    // === Botones DRS y OVERTAKE ===
    bool overtake_enabled = true;
    bool drs_enabled = false;

    btn_overtake = lv_btn_create(scr);
    lv_obj_set_size(btn_overtake, 140, 30);
    lv_obj_align(btn_overtake, LV_ALIGN_BOTTOM_LEFT, 10, -45);
    lv_obj_set_style_bg_color(btn_overtake,
        overtake_enabled ? lv_color_hex(0x00ff00) : lv_color_hex(0x444444), 0);
    lv_obj_set_style_border_width(btn_overtake, 0, 0);
    lv_obj_t *lbl_overtake = lv_label_create(btn_overtake);
    lv_label_set_text(lbl_overtake, "OVERTAKE");
    lv_obj_center(lbl_overtake);

    btn_drs = lv_btn_create(scr);
    lv_obj_set_size(btn_drs, 140, 30);
    lv_obj_align(btn_drs, LV_ALIGN_BOTTOM_RIGHT, -10, -45);
    lv_obj_set_style_bg_color(btn_drs,
        drs_enabled ? lv_color_hex(0x00ff00) : lv_color_hex(0xffcc00), 0);
    lv_obj_set_style_border_width(btn_drs, 0, 0);

    lv_obj_t *lbl_drs = lv_label_create(btn_drs);
    lv_label_set_text(lbl_drs, "DRS");
    lv_obj_center(lbl_drs);

    // === ERS ===
    int battery = 60;
    bar_batt = lv_bar_create(scr);
    lv_obj_set_size(bar_batt, 260, 12);
    lv_obj_align(bar_batt, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_bar_set_range(bar_batt, 0, 100);
    lv_bar_set_value(bar_batt, battery, LV_ANIM_ON);
    lv_obj_set_style_bg_color(bar_batt, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_grad_color(bar_batt, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_border_width(bar_batt, 0, 0);

    lbl_batt = lv_label_create(scr);
    char battery_txt[8];
    sprintf(battery_txt, "%d%%", battery);
    lv_label_set_text(lbl_batt, battery_txt);
    lv_obj_align(lbl_batt, LV_ALIGN_BOTTOM_RIGHT, -10, -25);
    lv_obj_set_style_text_color(lbl_batt, lv_color_white(), 0);
}