#ifndef UDP_TLV_PROTOCOL_H
#define UDP_TLV_PROTOCOL_H

#include <stdint.h>

#define UDP_TLV_PROTO_MAGIC 0x5352u /* 'SR' */
#define UDP_TLV_PROTO_MAJOR 1u
#define UDP_TLV_PROTO_MINOR 0u
#define UDP_TLV_MAX_EVENTS_PER_FRAME 16u
#define UDP_TLV_MAX_PAYLOAD_SIZE 128u

typedef enum {
    MSG_TYPE_INPUT_EVENTS = 1,
    MSG_TYPE_HEARTBEAT = 2,
    MSG_TYPE_ACK = 3
} msg_type_t;

typedef enum {
    EVENT_CLASS_BUTTON = 1,
    EVENT_CLASS_ENCODER = 2,
    EVENT_CLASS_SYSTEM = 3
} event_class_t;

typedef enum {
    VALUE_TYPE_BOOL = 1,
    VALUE_TYPE_I16 = 2,
    VALUE_TYPE_I32 = 3,
    VALUE_TYPE_F32 = 4
} value_type_t;

typedef enum {
    EVENT_ID_BTN_1 = 1,
    EVENT_ID_BTN_2 = 2,
    EVENT_ID_BTN_3 = 3,
    EVENT_ID_BTN_4 = 4,
    EVENT_ID_ENC_1_DELTA = 100,
    EVENT_ID_ENC_2_DELTA = 101,
    EVENT_ID_LINK_UPTIME_MS = 200
} event_id_t;

typedef enum {
    STATUS_OK = 0,
    STATUS_CRC_ERROR = 1,
    STATUS_BAD_VERSION = 2,
    STATUS_BAD_LENGTH = 3,
    STATUS_UNKNOWN_TYPE = 4
} status_code_t;

#pragma pack(push, 1)
typedef struct {
    uint16_t magic;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t msg_type;
    uint8_t flags;
    uint16_t seq;
    uint32_t timestamp_us;
    uint16_t payload_len;
    uint16_t header_crc;
} udp_tlv_header_t;

typedef struct {
    uint16_t event_id;
    uint8_t event_class;
    uint8_t value_type;
    uint8_t value_len;
} udp_tlv_event_prefix_t;
#pragma pack(pop)

#define UDP_TLV_HEADER_SIZE ((uint16_t)sizeof(udp_tlv_header_t))
#define UDP_TLV_EVENT_PREFIX_SIZE ((uint16_t)sizeof(udp_tlv_event_prefix_t))

/* CRC-16/IBM implementation, compact and portable for both SDKs. */
static inline uint16_t udp_tlv_crc16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFFu;
    uint16_t i = 0;
    while (i < len) {
        crc ^= data[i++];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 1u) {
                crc = (uint16_t)((crc >> 1) ^ 0xA001u);
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static inline const char *udp_tlv_event_name(uint16_t event_id) {
    switch (event_id) {
        case EVENT_ID_BTN_1: return "BTN_1";
        case EVENT_ID_BTN_2: return "BTN_2";
        case EVENT_ID_BTN_3: return "BTN_3";
        case EVENT_ID_BTN_4: return "BTN_4";
        case EVENT_ID_ENC_1_DELTA: return "ENC_1_DELTA";
        case EVENT_ID_ENC_2_DELTA: return "ENC_2_DELTA";
        case EVENT_ID_LINK_UPTIME_MS: return "LINK_UPTIME_MS";
        default: return "UNKNOWN_EVENT";
    }
}

static inline const char *udp_tlv_class_name(uint8_t event_class) {
    switch (event_class) {
        case EVENT_CLASS_BUTTON: return "BUTTON";
        case EVENT_CLASS_ENCODER: return "ENCODER";
        case EVENT_CLASS_SYSTEM: return "SYSTEM";
        default: return "UNKNOWN_CLASS";
    }
}

#endif
