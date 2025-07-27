/**
 * lv_conf.h - Configuración mínima para LVGL en Raspberry Pi Pico
 */
#ifndef LV_CONF_H
#define LV_CONF_H

/* Permite incluir los headers de forma simple */
#define LV_CONF_INCLUDE_SIMPLE 1

/* Resolución de tu pantalla */
#define LV_HOR_RES_MAX          320
#define LV_VER_RES_MAX          240

/* Sistema operativo (usamos sin OS en bare metal) */
#define LV_USE_OS              LV_OS_NONE

/* Caracteres de impresión */
#define LV_USE_PRINTF           1

/* Usa malloc/free del sistema */
#define LV_MEM_CUSTOM           0

/* Habilita los logs (opcional) */
#define LV_USE_LOG              1
#define LV_LOG_LEVEL            LV_LOG_LEVEL_WARN

#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_40 1

#endif // LV_CONF_H
