#include "usb_descriptors.h"

// Descriptor de Reporte HID completo (Basado en el estándar PID)
// Descriptor de Reporte HID completo (Basado en el estándar PID)
uint8_t const desc_hid_report[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)
        0x85, REPORT_ID_JOYSTICK,
        0x09, 0x01,    // Usage (Pointer)
        0xA1, 0x00,    // Collection (Physical)
            0x09, 0x30,    // Usage (X) - Volante
            0x16, 0x00, 0x80, // Logical Min (-32768)
            0x26, 0xFF, 0x7F, // Logical Max (32767)
            0x75, 0x10,    // Report Size 16
            0x95, 0x01,    // Report Count 1
            0x81, 0x02,    // Input (Data,Var,Abs)

            0x09, 0x31,    // Usage (Y) - Acelerador
            0x09, 0x32,    // Usage (Z) - Freno
            0x09, 0x35,    // Usage (Rz) - Embrague
            0x15, 0x00,    // Logical Min (0)
            0x26, 0xFF, 0xFF, // Logical Max (65535)
            0x75, 0x10,    // Report Size 16
            0x95, 0x03,    // Report Count 3
            0x81, 0x02,    // Input (Data,Var,Abs)
        0xC0,

        0x05, 0x09,    // Usage Page (Button)
        0x19, 0x01,    // Usage Minimum (Button 1)
        0x29, 0x08,    // Usage Maximum (Button 8)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,    // Report Size 1
        0x95, 0x08,    // Report Count 8
        0x81, 0x02,    // Input (Data,Var,Abs)

        // --- SECCIÓN PID (Force Feedback) ---
        0x05, 0x0F,    // Usage Page (PID)
        0x09, 0x01,    // Usage (Physical Interface Device)
        0xA1, 0x01,    // Collection (Application)
            
            // Reporte de Estado (Input)
            0x85, REPORT_ID_PID_STATE,
            0x09, 0x92, // PID State Report
            0xA1, 0x02, // Logical
                0x09, 0x9F, // Device Paused
                0x09, 0xA0, // Actuators Enabled
                0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x02,
                0x81, 0x02,
                0x95, 0x06, 0x81, 0x03, // Padding
            0xC0,

            // Set Effect (Output)
            0x85, REPORT_ID_SET_EFFECT,
            0x09, 0x21, // Set Effect Report
            0xA1, 0x02,
                0x09, 0x22, // Effect Block Index
                0x15, 0x01, 0x25, 0x28, 0x75, 0x08, 0x95, 0x01, 0x91, 0x02,
                0x09, 0x25, // Effect Type
                0x15, 0x01, 0x25, 0x0B, 0x91, 0x02,
            0xC0,

            // Device Control (Output)
            0x85, REPORT_ID_DEV_CONTROL,
            0x09, 0x96, // PID Device Control
            0xA1, 0x02,
                0x09, 0x97, // Enable Actuators
                0x09, 0x98, // Disable Actuators
                0x09, 0x99, // Stop All Effects
                0x09, 0x9A, // Reset
                0x15, 0x01, 0x25, 0x04, 0x75, 0x08, 0x95, 0x01, 0x91, 0x00,
            0xC0,

            // PID Pool (Feature) - Crítico para que el PC sepa cuántos efectos caben
            0x85, REPORT_ID_POOL_REPORT,
            0x09, 0x7F, // PID Pool Report
            0xA1, 0x02,
                0x09, 0x80, // RAM Pool Size (ej: 4096 bytes)
                0x15, 0x00, 0x26, 0x00, 0x10, 0x75, 0x10, 0x95, 0x01, 0xB1, 0x02,
                0x09, 0x83, // MaxSimultaneousEffects
                0x15, 0x00, 0x25, 0x28, 0x75, 0x08, 0x95, 0x01, 0xB1, 0x02,
            0xC0,

        0xC0, // End PID Collection
    0xC0 // End Joystick Collection
};

// --- Configuración de TinyUSB ---

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00, 
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0x044F, // Thrustmaster (Opcional: usar uno conocido ayuda a la compatibilidad)
    .idProduct          = 0xB66E, // T300RS (Ejemplo)
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN), 0, 100),
    TUD_HID_INOUT_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), 0x81, 0x01, 64, 1) // Polling 1ms para FFB
};

// Callbacks requeridos por TinyUSB
uint8_t const * tud_descriptor_device_cb(void) { return (uint8_t const *) &desc_device; }
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) { return desc_hid_report; }
uint8_t const * tud_descriptor_configuration_cb(uint8_t index) { return desc_configuration; }

// Strings
char const* string_desc_arr[] = { (const char[]) { 0x09, 0x04 }, "spg50", "Pico2 FFB", "123456" };
uint16_t _desc_str[32];
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    uint8_t chr_count;
    if (index == 0) { memcpy(&_desc_str[1], string_desc_arr[0], 2); chr_count = 1; }
    else {
        if (index >= 4) return NULL;
        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        for(uint8_t i=0; i<chr_count; i++) _desc_str[1+i] = str[i];
    }
    _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);
    return _desc_str;
}