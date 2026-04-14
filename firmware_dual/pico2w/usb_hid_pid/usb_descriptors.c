#include "tusb.h"

#define STRID_LANGID 0
#define STRID_MANUFACTURER 1
#define STRID_PRODUCT 2
#define STRID_SERIAL 3

// Descriptor de Dispositivo
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = 0xCAFE,
    .idProduct          = 0x0002,
    .bcdDevice          = 0x0100,
    .iManufacturer      = STRID_MANUFACTURER,
    .iProduct           = STRID_PRODUCT,
    .iSerialNumber      = STRID_SERIAL,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

// HID Report Descriptor (Gamepad + PID)
uint8_t const desc_hid_report[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x04,        // Usage (Joystick) <--- Cambiado de Gamepad a Joystick
    0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,    // REPORT ID 1 (Input: Pico -> PC)
        0x09, 0x01,    // Usage (Pointer)
        0xA1, 0x00,    // Collection (Physical)
            0x09, 0x30, // Usage (X)
            0x09, 0x31, // Usage (Y)
            0x15, 0x81, // Logical Minimum (-127)
            0x25, 0x7F, // Logical Maximum (127)
            0x75, 0x08, // Report Size (8 bits por eje)
            0x95, 0x02, // Report Count (2 ejes: X e Y)
            0x81, 0x02, // Input (Data, Var, Abs)
        0xC0,          // End Collection (Physical)

        0x05, 0x09,    // Usage Page (Button)
        0x19, 0x01,    // Usage Minimum (Button 1)
        0x29, 0x08,    // Usage Maximum (Button 8)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,    // Report Size (1 bit)
        0x95, 0x08,    // Report Count (8 botones)
        0x81, 0x02,    // Input (Data, Var, Abs)

    // --- BLOQUE PID (FORCE FEEDBACK) ---
    // Mantenemos el bloque PID que ya teníamos abajo...
        0x05, 0x0F,    // Usage Page (PID)
        0x09, 0x01,    // Usage (Physical Interface Device)
        0xA1, 0x01,    // Collection (Application)
            0x85, 0x02, // REPORT ID 2 (Output: PC -> Pico)
            0x09, 0x21, // Set Effect Report
            0xA1, 0x02, 
                0x09, 0x22, 0x15, 0x01, 0x25, 0x7F, 0x75, 0x08, 0x95, 0x01, 0x91, 0x02,
            0xC0,
        0xC0,
    0xC0               // End Collection (Application)
};

uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
    return desc_hid_report;
}

// Config Descriptor
uint8_t const desc_configuration[] = {
    // Config Descriptor (9 bytes)
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN), 0, 100),

    // HID Interface Descriptor con IN y OUT endpoints
    // Interfaz 0, String 0, Protocolo None, Largo del Report Descriptor
    // Endpoint IN: 0x81, Endpoint OUT: 0x01 (Este es el que falta)
    // Tamaño 64, intervalo 5ms
    TUD_HID_INOUT_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), 0x81, 0x01, CFG_TUD_HID_EP_BUFSIZE, 5)
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    return desc_configuration;
}

// Strings
char const* string_desc_arr[] = { (const char[]) { 0x09, 0x04 }, "Alexis", "Pico2 FFB Sniffer", "123456" };
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