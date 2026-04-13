#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "secrets.h"
#include "udp_tlv_protocol.h"

#define UDP_LISTEN_PORT 7777
#define BEACON_TARGET "255.255.255.255"

static volatile uint32_t g_frames_ok = 0;
static volatile uint32_t g_frames_bad = 0;

static void log_event(const udp_tlv_event_prefix_t *prefix, const uint8_t *value_bytes) {
    if (prefix->value_type == VALUE_TYPE_BOOL && prefix->value_len == 1) {
        printf("event=%u(%s) class=%s value=%u\n",
               prefix->event_id,
               udp_tlv_event_name(prefix->event_id),
               udp_tlv_class_name(prefix->event_class),
               value_bytes[0]);
        return;
    }

    if (prefix->value_type == VALUE_TYPE_I16 && prefix->value_len == 2) {
        int16_t value_i16;
        memcpy(&value_i16, value_bytes, sizeof(value_i16));
        printf("event=%u(%s) class=%s value=%d\n",
               prefix->event_id,
               udp_tlv_event_name(prefix->event_id),
               udp_tlv_class_name(prefix->event_class),
               value_i16);
        return;
    }

    if (prefix->value_type == VALUE_TYPE_I32 && prefix->value_len == 4) {
        int32_t value_i32;
        memcpy(&value_i32, value_bytes, sizeof(value_i32));
        printf("event=%u(%s) class=%s value=%ld\n",
               prefix->event_id,
               udp_tlv_event_name(prefix->event_id),
               udp_tlv_class_name(prefix->event_class),
               (long)value_i32);
        return;
    }

    printf("event=%u(%s) class=%s value_type=%u len=%u\n",
           prefix->event_id,
           udp_tlv_event_name(prefix->event_id),
           udp_tlv_class_name(prefix->event_class),
           prefix->value_type,
           prefix->value_len);
}

static bool parse_payload(const uint8_t *payload, uint16_t payload_len) {
    if (payload_len < 1) {
        return false;
    }

    uint8_t event_count = payload[0];
    uint16_t offset = 1;

    for (uint8_t i = 0; i < event_count; i++) {
        udp_tlv_event_prefix_t prefix;

        if (offset + UDP_TLV_EVENT_PREFIX_SIZE > payload_len) {
            return false;
        }

        memcpy(&prefix, payload + offset, UDP_TLV_EVENT_PREFIX_SIZE);
        offset += UDP_TLV_EVENT_PREFIX_SIZE;

        if (offset + prefix.value_len > payload_len) {
            return false;
        }

        log_event(&prefix, payload + offset);
        offset += prefix.value_len;
    }

    return offset == payload_len;
}

static bool parse_frame(const uint8_t *data, uint16_t len) {
    udp_tlv_header_t header;
    udp_tlv_header_t header_check;

    if (len < UDP_TLV_HEADER_SIZE) {
        return false;
    }

    memcpy(&header, data, UDP_TLV_HEADER_SIZE);

    if (header.magic != UDP_TLV_PROTO_MAGIC || header.version_minor != UDP_TLV_PROTO_MAJOR) {
        return false;
    }

    if (header.payload_len != (uint16_t)(len - UDP_TLV_HEADER_SIZE)) {
        return false;
    }

    memcpy(&header_check, &header, sizeof(header_check));
    header_check.header_crc = 0;
    if (udp_tlv_crc16((const uint8_t *)&header_check, UDP_TLV_HEADER_SIZE) != header.header_crc) {
        return false;
    }

    if (header.msg_type == MSG_TYPE_INPUT_EVENTS) {
        return parse_payload(data + UDP_TLV_HEADER_SIZE, header.payload_len);
    }

    return false;
}

static void udp_rx_callback(void *arg,
                            struct udp_pcb *pcb,
                            struct pbuf *p,
                            const ip_addr_t *addr,
                            u16_t port) {
    (void)arg;
    (void)pcb;
    (void)addr;
    (void)port;

    if (p == NULL) {
        return;
    }
    printf("Received UDP packet from %s:%u len=%u\n", ipaddr_ntoa(addr), port, p->tot_len);
    if (p->tot_len <= (UDP_TLV_HEADER_SIZE + UDP_TLV_MAX_PAYLOAD_SIZE)) {
        uint8_t frame[UDP_TLV_HEADER_SIZE + UDP_TLV_MAX_PAYLOAD_SIZE];
        if (pbuf_copy_partial(p, frame, p->tot_len, 0) == p->tot_len) {
            if (parse_frame(frame, p->tot_len)) {
                g_frames_ok++;
            } else {
                g_frames_bad++;
            }
        } else {
            g_frames_bad++;
        }
    } else {
        g_frames_bad++;
    }

    pbuf_free(p);
}

int main(void) {
    stdio_init_all();
    
    if (cyw43_arch_init()) {
        printf("wifi init failed\n");
        return -1;
    }

    cyw43_arch_enable_ap_mode(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    ip4_addr_t ap_ip;
    ip4_addr_t ap_mask;
    ip4_addr_t ap_gw;
    IP4_ADDR(&ap_ip, 192, 168, 4, 1);
    IP4_ADDR(&ap_mask, 255, 255, 255, 0);
    IP4_ADDR(&ap_gw, 192, 168, 4, 1);

    cyw43_arch_lwip_begin();
    netif_set_addr(&cyw43_state.netif[CYW43_ITF_AP], &ap_ip, &ap_mask, &ap_gw);
    cyw43_arch_lwip_end();

    printf("AP started ssid=%s ip=%s port=%d\n",
        WIFI_SSID,
        ip4addr_ntoa(netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_AP])),
        UDP_LISTEN_PORT
    );
    

    cyw43_arch_lwip_begin();
    struct udp_pcb *pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (pcb == NULL) {
        cyw43_arch_lwip_end();
        printf("udp pcb create failed\n");
        return -1;
    }

    err_t bind_err = udp_bind(pcb, IP_ANY_TYPE, UDP_LISTEN_PORT);
    if (bind_err != ERR_OK) {
        udp_remove(pcb);
        cyw43_arch_lwip_end();
        printf("udp bind failed %d\n", (int)bind_err);
        return -1;
    }
    udp_recv(pcb, udp_rx_callback, NULL);
    cyw43_arch_lwip_end();

    while (true) {
        printf("rx_ok=%lu rx_bad=%lu\n\n", (unsigned long)g_frames_ok, (unsigned long)g_frames_bad);
        sleep_ms(1000);
    }

    return 0;
}
