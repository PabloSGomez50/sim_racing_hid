#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "udp_tlv_protocol.h"
#include "wifi_config.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_MAX_RETRY 10

static const char *TAG = "udp_sta";
static EventGroupHandle_t wifi_event_group;
static uint8_t s_retry_num = 0;
static uint16_t s_seq = 0;
static esp_netif_t *s_sta_netif = NULL;

static uint16_t write_event_tlv(uint8_t *dst,
                                uint16_t max_len,
                                uint16_t event_id,
                                uint8_t event_class,
                                uint8_t value_type,
                                const void *value,
                                uint8_t value_len) {
    udp_tlv_event_prefix_t prefix;

    if (max_len < UDP_TLV_EVENT_PREFIX_SIZE + value_len) {
        return 0;
    }

    prefix.event_id = event_id;
    prefix.event_class = event_class;
    prefix.value_type = value_type;
    prefix.value_len = value_len;

    memcpy(dst, &prefix, UDP_TLV_EVENT_PREFIX_SIZE);
    memcpy(dst + UDP_TLV_EVENT_PREFIX_SIZE, value, value_len);
    return (uint16_t)(UDP_TLV_EVENT_PREFIX_SIZE + value_len);
}

static uint16_t build_input_frame(uint8_t *buffer, uint16_t buffer_len) {
    udp_tlv_header_t header;
    uint16_t offset = UDP_TLV_HEADER_SIZE;
    uint8_t *payload = buffer + UDP_TLV_HEADER_SIZE;
    uint8_t event_count = 0;
    bool button = ((xTaskGetTickCount() / pdMS_TO_TICKS(500)) % 2) != 0;
    int16_t enc_delta = (int16_t)((esp_random() % 3) - 1);
    uint32_t uptime_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    uint16_t written;

    if (buffer_len < UDP_TLV_HEADER_SIZE + 1) {
        return 0;
    }

    payload[0] = 0;
    offset += 1;

    written = write_event_tlv(buffer + offset,
                              (uint16_t)(buffer_len - offset),
                              EVENT_ID_BTN_1,
                              EVENT_CLASS_BUTTON,
                              VALUE_TYPE_BOOL,
                              &button,
                              sizeof(button));
    if (written > 0) {
        offset += written;
        event_count++;
    }

    written = write_event_tlv(buffer + offset,
                              (uint16_t)(buffer_len - offset),
                              EVENT_ID_ENC_1_DELTA,
                              EVENT_CLASS_ENCODER,
                              VALUE_TYPE_I16,
                              &enc_delta,
                              sizeof(enc_delta));
    if (written > 0) {
        offset += written;
        event_count++;
    }

    written = write_event_tlv(buffer + offset,
                              (uint16_t)(buffer_len - offset),
                              EVENT_ID_LINK_UPTIME_MS,
                              EVENT_CLASS_SYSTEM,
                              VALUE_TYPE_I32,
                              &uptime_ms,
                              sizeof(uptime_ms));
    if (written > 0) {
        offset += written;
        event_count++;
    }

    payload[0] = event_count;

    header.magic = UDP_TLV_PROTO_MAGIC;
    header.version_major = UDP_TLV_PROTO_MAJOR;
    header.version_minor = UDP_TLV_PROTO_MINOR;
    header.msg_type = MSG_TYPE_INPUT_EVENTS;
    header.flags = 0;
    header.seq = s_seq++;
    header.timestamp_us = (uint32_t)(esp_timer_get_time() & 0xFFFFFFFFu);
    header.payload_len = (uint16_t)(offset - UDP_TLV_HEADER_SIZE);
    header.header_crc = 0;

    memcpy(buffer, &header, UDP_TLV_HEADER_SIZE);
    header.header_crc = udp_tlv_crc16(buffer, UDP_TLV_HEADER_SIZE);
    memcpy(buffer, &header, UDP_TLV_HEADER_SIZE);

    return offset;
}

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data) {
    (void)arg;
    (void)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "sta connected to AP");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "retry to connect to AP");
        } else {
            ESP_LOGE(TAG, "max retry reached");
        }
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        s_retry_num = 0;
        ESP_LOGI(TAG, "connected, ip=" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t wifi_init_sta(void) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
            .ssid = UDP_WIFI_SSID,
            .password = UDP_WIFI_PASS, 
        },
    };

    // snprintf((char *)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", UDP_WIFI_SSID);
    // snprintf((char *)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", UDP_WIFI_PASS);

    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_sta_netif = esp_netif_create_default_wifi_sta();
    if (s_sta_netif == NULL) {
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Pico AP is configured with fixed 192.168.4.1, so use static STA IP to avoid DHCP dependency. */
    esp_err_t dhcp_stop_err = esp_netif_dhcpc_stop(s_sta_netif);
    if (dhcp_stop_err != ESP_OK && dhcp_stop_err != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED) {
        return dhcp_stop_err;
    }
    esp_netif_ip_info_t ip_info;
    ip_info.ip.addr = ipaddr_addr(UDP_STA_IP);
    ip_info.gw.addr = ipaddr_addr(UDP_STA_GATEWAY);
    ip_info.netmask.addr = ipaddr_addr(UDP_STA_NETMASK);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(s_sta_netif, &ip_info));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG,
             "static ip configured ip=%s gw=%s mask=%s",
             UDP_STA_IP,
             UDP_STA_GATEWAY,
             UDP_STA_NETMASK);

    return ESP_OK;
}

static void udp_sender_task(void *arg) {
    (void)arg;
    struct sockaddr_in dest_addr = {0};
    int sock = -1;
    uint8_t frame[UDP_TLV_HEADER_SIZE + UDP_TLV_MAX_PAYLOAD_SIZE];

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(UDP_REMOTE_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(UDP_REMOTE_IP);
    
    while (true) {
        EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                               WIFI_CONNECTED_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               pdMS_TO_TICKS(1000));
        if ((bits & WIFI_CONNECTED_BIT) == 0) {
            ESP_LOGW(TAG, "waiting for wifi connection...");
            continue;
        }

        if (sock < 0) {
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
            if (sock < 0) {
                ESP_LOGE(TAG, "cannot create socket");
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
            ESP_LOGI(TAG, "UDP socket ready -> %s:%d", UDP_REMOTE_IP, UDP_REMOTE_PORT);
        }

        uint16_t frame_len = build_input_frame(frame, sizeof(frame));
        if (frame_len == 0) {
            ESP_LOGE(TAG, "failed to serialize frame");
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        int sent = sendto(sock,
                          frame,
                          frame_len,
                          0,
                          (struct sockaddr *)&dest_addr,
                          sizeof(dest_addr));
        if (sent < 0) {
            ESP_LOGE(TAG, "send error");
            close(sock);
            sock = -1;
        }
        ESP_LOGI(TAG, "frame sent, len=%d", sent);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(wifi_init_sta());

    xTaskCreate(udp_sender_task, "udp_sender_task", 4096, NULL, 5, NULL);
}