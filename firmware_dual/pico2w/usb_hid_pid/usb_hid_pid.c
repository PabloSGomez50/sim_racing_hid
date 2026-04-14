#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

// --- MODIFICACIÓN 1: Consistencia de IDs ---
// En tus descriptores anteriores, el Gamepad era el ID 4. 
// Usaremos el 4 para que coincida con el script de Python.
#define REPORT_ID_GAMEPAD 1
#define REPORT_ID_PID 2
#define LED_PIN PICO_DEFAULT_LED_PIN

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
    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        print_ffb_raw(report_id, buffer, bufsize);
    }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, 
                                hid_report_type_t report_type, 
                                uint8_t* buffer, uint16_t reqlen) {
    (void) instance; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
    return 0;
}

void send_hid_report() {
    if (!tud_hid_ready()) return;

    static uint32_t start_ms = 0;
    uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());

    if (current_time_ms - start_ms < 10) return; 
    start_ms = current_time_ms;

    static int8_t pos = 0;
    static int8_t direction = 1;
    pos += direction;
    if (pos >= 100 || pos <= -100) direction *= -1;

    simple_report_t report = {
        .x = pos,
        .y = 0,
        .buttons = 0x00
    };

    // --- MODIFICACIÓN 3: Uso del ID correcto ---
    tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}

int main() {
    stdio_init_all();
    
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