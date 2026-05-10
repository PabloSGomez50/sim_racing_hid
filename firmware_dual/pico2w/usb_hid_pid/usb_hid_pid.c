#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "tusb.h"
#include "usb_descriptors.h"

// --- MODIFICACIÓN 1: Consistencia de IDs ---
// En tus descriptores anteriores, el Gamepad era el ID 4. 
// Usaremos el 4 para que coincida con el script de Python.
#define REPORT_ID_GAMEPAD 1
#define REPORT_ID_PID 2
#define LED_PIN PICO_DEFAULT_LED_PIN

#define ADC_CH_PIN 26
#define ADC_BRAKE_CH 0
#define ADC_THROTTLE_CH 1
#define ADC_STEERING_CH 2

typedef struct {
    int8_t x;       
    int8_t y;       
    uint8_t buttons; 
} simple_report_t;

// --- MODIFICACIÓN 2: Feedback Visual ---
// Función para parpadear el LED cuando llega un comando FFB
void signal_reception() {
    gpio_put(LED_PIN, 1);
    sleep_ms(10); // Pulso corto
    gpio_put(LED_PIN, 0);
}

static uint8_t get_effective_report_id(uint8_t report_id, uint8_t const* buffer, uint16_t bufsize) {
    // Some hosts/libraries deliver report_id=0 and include the actual Report ID in buffer[0].
    if (report_id == 0 && bufsize > 0) {
        return buffer[0];
    }
    return report_id;
}

void print_ffb_raw(uint8_t report_id, uint8_t const* buffer, uint16_t bufsize) {
    uint8_t effective_id = get_effective_report_id(report_id, buffer, bufsize);

    printf("[HID RECEIVE] raw_id=0x%02X effective_id=0x%02X | Bytes:", report_id, effective_id);
    for (uint16_t i = 0; i < bufsize; i++) {
        printf(" %02X", buffer[i]);
    }
    printf(" | Total: %u bytes\n", bufsize);

    if (effective_id == REPORT_ID_PID) {
        signal_reception();
    }
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, 
                            hid_report_type_t report_type, 
                            uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;
    if (report_type == HID_REPORT_TYPE_FEATURE) {
        if (report_id == REPORT_ID_POOL_REPORT) {
            // El juego pregunta por la memoria disponible para efectos
            uint8_t response[3] = {0}; // Aquí puedes definir tu memoria total y simultánea
            response[0] = 0x00; // RAM Pool Size (ej: 4096 bytes -> 0x1000)
            response[1] = 0x10;
            response[2] = 0x00; // Max Simultaneous Effects (ej: 40 -> 0x28)
            tud_hid_report(REPORT_ID_POOL_REPORT, response, sizeof(response));
        }
    }
    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        print_ffb_raw(report_id, buffer, bufsize);
        switch(report_id) {
            case REPORT_ID_SET_EFFECT:
                // El juego está definiendo un efecto (Constant, Spring, etc.)
                printf("[HID RECEIVE] Set Effect Command Received\n");
                if (bufsize < 8) {
                    printf("Error: Set Effect report too short (%u bytes)\n", bufsize);
                    return;
                }
                printf("Effect Block Index: %d\n", buffer[0]);
                printf("Effect Type: %d\n", buffer[1]);
                printf("Duration: %d\n", ((uint16_t)buffer[2] << 8) | buffer[3]);
                break;
            case REPORT_ID_DEV_CONTROL:
                // El juego activó/desactivó el FFB
                printf("[HID RECEIVE] Device Control Command Received\n");
                break;
        }
    }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, 
                                hid_report_type_t report_type, 
                                uint8_t* buffer, uint16_t reqlen) {
    (void) instance;
    (void) buffer;
    (void) reqlen;
    if (report_type == HID_REPORT_TYPE_FEATURE && report_id == REPORT_ID_POOL_REPORT) {
        printf("[HID GET_REPORT] Report ID: 0x%02X (PID Pool Report)\n", report_id);
        pid_pool_report_t pool = {
            .ram_pool_size = 0x0FFF, // 4KB de "memoria" simulada
            .max_simultaneous_effects = 40,
            .memory_management = 3 // Device Managed + Shared Parameter Blocks
        };
        memcpy(buffer, &pool, sizeof(pool));
        return sizeof(pool);
    }
    return 0;
}

int16_t read_adc_ch(uint8_t ch) {
    adc_select_input(ch);
    // return adc_read();
    return ((int16_t)adc_read() - 2048) << 4; // Centrar en 0 para facilitar el manejo de ejes (rango -2048 a +2047)
}

void send_hid_report() {
    if (!tud_hid_ready()) return;

    static uint32_t start_ms = 0;
    uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());

    if (current_time_ms - start_ms < 10) return; 
    start_ms = current_time_ms;

    // static int8_t pos = 0;
    // static int8_t direction = 1;
    // pos += direction;
    // if (pos >= 100 || pos <= -100) direction *= -1;


    joystick_report_t report = {
        .steering = read_adc_ch(ADC_STEERING_CH), // Escalamos a rango completo
        .accelerator = read_adc_ch(ADC_THROTTLE_CH),
        .brake = read_adc_ch(ADC_BRAKE_CH),
        .clutch = 0,
        .buttons = 0x00
    };

    // --- MODIFICACIÓN 3: Uso del ID correcto ---
    tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}

int main() {
    stdio_init_all();
    adc_init();
    adc_gpio_init(ADC_CH_PIN + ADC_BRAKE_CH);
    adc_gpio_init(ADC_CH_PIN + ADC_THROTTLE_CH);
    adc_gpio_init(ADC_CH_PIN + ADC_STEERING_CH);
    adc_select_input(ADC_BRAKE_CH);

    
    // Inicializar LED para debug físico
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    tusb_init();

    printf("\n--- Pico2 FFB Tester Ready ---\n");
    printf("Usando Report ID: %d\n", REPORT_ID_GAMEPAD);

    while (1) {
        tud_task();        
        send_hid_report(); 
    }
    return 0;
}