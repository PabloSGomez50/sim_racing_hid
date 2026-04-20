#ifndef __USB_DESCRIPTORS_H__
#define __USB_DESCRIPTORS_H__

#include "tusb.h"
#include "stdint.h"
#include "ffb_reports.h"

/* IDs de los Reportes HID */
enum {
    REPORT_ID_JOYSTICK = 1,
    REPORT_ID_PID_STATE,      // Input
    REPORT_ID_SET_EFFECT,     // Output
    REPORT_ID_SET_CONST,      // Output
    REPORT_ID_OP_EFFECT,      // Output
    REPORT_ID_DEV_CONTROL,    // Output
    REPORT_ID_BLOCK_FREE,     // Output
    REPORT_ID_DEV_GAIN,       // Output
    REPORT_ID_CREATE_NEW,     // Feature
    REPORT_ID_POOL_REPORT     // Feature
};

#endif // __USB_DESCRIPTORS_H__