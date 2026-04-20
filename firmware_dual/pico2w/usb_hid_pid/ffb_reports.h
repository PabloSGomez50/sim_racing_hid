#ifndef FFB_REPORTS_H
#define FFB_REPORTS_H

#include <stdint.h>

// Estructura del reporte de entrada (Pico -> PC)
// Coincide con el Report ID 0x01 del descriptor
typedef struct __attribute__((packed)) {
int16_t  steering;  // Eje X: -32768 a 32767
uint16_t accelerator;
uint16_t brake;
uint16_t clutch;
uint8_t  buttons;
} joystick_report_t;

// Reporte de Estado PID (Pico -> PC)
// Coincide con el Report ID 0x02
typedef struct __attribute__((packed)) {
uint8_t status; // bits: 0=DevicePaused, 1=ActuatorsEnabled, 2=SafetySwitch, 3=ActuatorOverride
uint8_t effect_block_index; // Índice del efecto que se está ejecutando
} pid_state_report_t;

// --- REPORTES DE FEATURE (Configuración) ---

// Report ID 0x07: PID Pool Report (Respuesta al "GET_REPORT" de memoria)
typedef struct __attribute__((packed)) {
uint16_t ram_pool_size;          // Tamaño total de memoria (ej: 0x0400 para 1KB)
uint8_t  max_simultaneous_effects; // Cuántos efectos puede procesar a la vez (ej: 40)
uint8_t  memory_management;       // 0=DeviceManaged, 1=Shared
} pid_pool_report_t;

// --- REPORTES DE SALIDA (PC -> Pico) ---
// Estas estructuras te servirán para parsear los datos en tud_hid_set_report_cb
typedef struct __attribute__((packed)){
uint8_t effect_block_index;
uint8_t effect_type; // 1: Constant, 2: Ramp, 3: Square, etc.
uint16_t duration;
uint16_t trigger_repeat_interval;
uint8_t gain;
uint8_t trigger_button;
} set_effect_report_t;

typedef struct __attribute__((packed)){
uint8_t effect_block_index;
int16_t magnitude; // Fuerza constante
} set_constant_force_report_t;

#endif // FFB_REPORTS_H