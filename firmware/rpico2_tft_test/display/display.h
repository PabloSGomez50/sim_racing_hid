#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "ili9341.h"
#include "lvgl.h"
#include "config.h"
#include "assets/Logo_Williams_F1_transparent.h"

// Inicializa el display y la interfaz LVGL
void display_lvgl_init(void);
void f1_dashboard_create(void);

#endif // DISPLAY_H
