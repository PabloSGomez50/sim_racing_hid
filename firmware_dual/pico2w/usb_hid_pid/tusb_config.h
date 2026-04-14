#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

// El SDK ya define CFG_TUSB_MCU, por eso lanzaba el warning de "redefined".
// Lo envolvemos en un ifndef por seguridad.
#ifndef CFG_TUSB_MCU
  #define CFG_TUSB_MCU                OPT_MCU_RP2350
#endif

// Definición crítica que faltaba: Configura el puerto USB 0 como Device
#define CFG_TUSB_RHPORT0_MODE         OPT_MODE_DEVICE

#define CFG_TUD_ENABLED               1
#define CFG_TUD_HID                   1
#define CFG_TUD_HID_EP_BUFSIZE        64

#endif
