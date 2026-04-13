#include "input_buttons.h"

#include <stddef.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "udp_tlv_protocol.h"

#define BTN_DEBOUNCE_US 15000
#define BTN_ISR_QUEUE_LEN 32

typedef struct {
    uint8_t index;
    gpio_num_t pin;
    uint16_t event_id;
} button_desc_t;

typedef struct {
    uint8_t button_idx;
    int64_t isr_time_us;
} button_isr_evt_t;

/* Active-low buttons with pull-up enabled. */
static button_desc_t s_buttons[] = {
    { .index = 0, .pin = GPIO_NUM_4, .event_id = EVENT_ID_BTN_1 },
    { .index = 1, .pin = GPIO_NUM_16, .event_id = EVENT_ID_BTN_2 },
    { .index = 2, .pin = GPIO_NUM_17, .event_id = EVENT_ID_BTN_3 },
};

#define BUTTON_COUNT ((uint8_t)(sizeof(s_buttons) / sizeof(s_buttons[0])))

static const char *TAG = "input_buttons";
static QueueHandle_t s_button_isr_queue = NULL;
static uint32_t s_button_state_mask = 0;
static int64_t s_button_last_change_us[BUTTON_COUNT] = {0};

static void IRAM_ATTR button_gpio_isr(void *arg) {
    const button_desc_t *button = (const button_desc_t *)arg;
    button_isr_evt_t evt;
    BaseType_t hp_task_woken = pdFALSE;

    if (button == NULL) {
        return;
    }

    evt.button_idx = button->index;
    evt.isr_time_us = esp_timer_get_time();

    if (s_button_isr_queue != NULL) {
        xQueueSendFromISR(s_button_isr_queue, &evt, &hp_task_woken);
        if (hp_task_woken == pdTRUE) {
            portYIELD_FROM_ISR();
        }
    }
}

esp_err_t input_buttons_init(void) {
    gpio_config_t io_cfg = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 0,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };

    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        io_cfg.pin_bit_mask |= (1ULL << s_buttons[i].pin);
    }

    esp_err_t err = gpio_config(&io_cfg);
    if (err != ESP_OK) {
        return err;
    }

    s_button_isr_queue = xQueueCreate(BTN_ISR_QUEUE_LEN, sizeof(button_isr_evt_t));
    if (s_button_isr_queue == NULL) {
        return ESP_ERR_NO_MEM;
    }

    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        bool pressed = (gpio_get_level(s_buttons[i].pin) == 0);
        if (pressed) {
            s_button_state_mask |= (1UL << i);
        }

        err = gpio_isr_handler_add(s_buttons[i].pin, button_gpio_isr, (void *)&s_buttons[i]);
        if (err != ESP_OK) {
            return err;
        }
    }

    ESP_LOGI(TAG, "buttons initialized count=%u", BUTTON_COUNT);
    return ESP_OK;
}

void input_buttons_poll(void) {
    button_isr_evt_t evt;

    if (s_button_isr_queue == NULL) {
        return;
    }

    while (xQueueReceive(s_button_isr_queue, &evt, 0) == pdTRUE) {
        if (evt.button_idx >= BUTTON_COUNT) {
            continue;
        }

        if ((evt.isr_time_us - s_button_last_change_us[evt.button_idx]) < BTN_DEBOUNCE_US) {
            continue;
        }

        if (gpio_get_level(s_buttons[evt.button_idx].pin) == 0) {
            s_button_state_mask |= (1UL << evt.button_idx);
        } else {
            s_button_state_mask &= ~(1UL << evt.button_idx);
        }

        s_button_last_change_us[evt.button_idx] = evt.isr_time_us;
    }
}

uint8_t input_buttons_count(void) {
    return BUTTON_COUNT;
}

bool input_buttons_get_state(uint8_t idx) {
    if (idx >= BUTTON_COUNT) {
        return false;
    }
    return ((s_button_state_mask >> idx) & 0x1U) != 0;
}

uint16_t input_buttons_get_event_id(uint8_t idx) {
    if (idx >= BUTTON_COUNT) {
        return 0;
    }
    return s_buttons[idx].event_id;
}
