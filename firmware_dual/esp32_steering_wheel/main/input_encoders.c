#include "input_encoders.h"

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"

#include "udp_tlv_protocol.h"

/*
 * Change these pins to match your hardware routing.
 * They are left as compile-time defaults to keep this module reusable.
 */
#ifndef ENC_1_PIN_A
#define ENC_1_PIN_A GPIO_NUM_32
#endif
#ifndef ENC_1_PIN_B
#define ENC_1_PIN_B GPIO_NUM_33
#endif
#ifndef ENC_2_PIN_A
#define ENC_2_PIN_A GPIO_NUM_25
#endif
#ifndef ENC_2_PIN_B
#define ENC_2_PIN_B GPIO_NUM_26
#endif

/*
 * Raw quadrature decoding can report multiple counts per physical detent.
 * Tune this divisor to convert raw transitions to logical "1 step" events.
 */
#ifndef ENCODER_COUNTS_PER_STEP
#define ENCODER_COUNTS_PER_STEP 2
#endif

#if ENCODER_COUNTS_PER_STEP < 1
#error "ENCODER_COUNTS_PER_STEP must be >= 1"
#endif

typedef struct {
    gpio_num_t pin_a;
    gpio_num_t pin_b;
    uint16_t event_id;
    pcnt_unit_handle_t unit;
    pcnt_channel_handle_t chan_a;
    pcnt_channel_handle_t chan_b;
    int16_t last_count;
    int16_t substep_remainder;
    int16_t delta_acc;
} encoder_desc_t;

static encoder_desc_t s_encoders[] = {
    { .pin_a = ENC_1_PIN_A, .pin_b = ENC_1_PIN_B, .event_id = EVENT_ID_ENC_1_DELTA },
    // { .pin_a = ENC_2_PIN_A, .pin_b = ENC_2_PIN_B, .event_id = EVENT_ID_ENC_2_DELTA },
};

#define ENCODER_COUNT ((uint8_t)(sizeof(s_encoders) / sizeof(s_encoders[0])))

static const char *TAG = "input_encoders";

static int16_t saturating_add_i16(int16_t a, int16_t b) {
    int32_t sum = (int32_t)a + (int32_t)b;
    if (sum > INT16_MAX) {
        return INT16_MAX;
    }
    if (sum < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)sum;
}

static esp_err_t encoder_init(encoder_desc_t *enc) {
    pcnt_unit_config_t unit_config = {
        .low_limit = INT16_MIN,
        .high_limit = INT16_MAX,
    };
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = enc->pin_a,
        .level_gpio_num = enc->pin_b,
    };
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = enc->pin_b,
        .level_gpio_num = enc->pin_a,
    };
    pcnt_glitch_filter_config_t filter = {
        .max_glitch_ns = 1000,
    };

    esp_err_t err = pcnt_new_unit(&unit_config, &enc->unit);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_unit_set_glitch_filter(enc->unit, &filter);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_new_channel(enc->unit, &chan_a_config, &enc->chan_a);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_new_channel(enc->unit, &chan_b_config, &enc->chan_b);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_channel_set_edge_action(enc->chan_a,
                                       PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                       PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    if (err != ESP_OK) {
        return err;
    }
    err = pcnt_channel_set_level_action(enc->chan_a,
                                        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                        PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_channel_set_edge_action(enc->chan_b,
                                       PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                                       PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    if (err != ESP_OK) {
        return err;
    }
    err = pcnt_channel_set_level_action(enc->chan_b,
                                        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                        PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_unit_enable(enc->unit);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_unit_clear_count(enc->unit);
    if (err != ESP_OK) {
        return err;
    }

    err = pcnt_unit_start(enc->unit);
    if (err != ESP_OK) {
        return err;
    }

    enc->last_count = 0;
    enc->substep_remainder = 0;
    enc->delta_acc = 0;
    return ESP_OK;
}

esp_err_t input_encoders_init(void) {
    for (uint8_t i = 0; i < ENCODER_COUNT; i++) {
        esp_err_t err = encoder_init(&s_encoders[i]);
        if (err != ESP_OK) {
            return err;
        }
        ESP_LOGI(TAG,
                 "encoder[%u] initialized A=%d B=%d event_id=%u",
                 i,
                 (int)s_encoders[i].pin_a,
                 (int)s_encoders[i].pin_b,
                 s_encoders[i].event_id);
    }
    return ESP_OK;
}

void input_encoders_poll(void) {
    for (uint8_t i = 0; i < ENCODER_COUNT; i++) {
        int count = 0;
        if (pcnt_unit_get_count(s_encoders[i].unit, &count) != ESP_OK) {
            continue;
        }

        int16_t current = (int16_t)count;
        int16_t raw_delta = (int16_t)(current - s_encoders[i].last_count);
        s_encoders[i].last_count = current;

        int32_t accum = (int32_t)s_encoders[i].substep_remainder + (int32_t)raw_delta;
        int16_t step_delta = (int16_t)(accum / ENCODER_COUNTS_PER_STEP);
        s_encoders[i].substep_remainder = (int16_t)(accum % ENCODER_COUNTS_PER_STEP);

        s_encoders[i].delta_acc = saturating_add_i16(s_encoders[i].delta_acc, step_delta);
    }
}

uint8_t input_encoders_count(void) {
    return ENCODER_COUNT;
}

int16_t input_encoders_get_delta_and_clear(uint8_t idx) {
    if (idx >= ENCODER_COUNT) {
        return 0;
    }

    int16_t delta = s_encoders[idx].delta_acc;
    s_encoders[idx].delta_acc = 0;
    return delta;
}

uint16_t input_encoders_get_event_id(uint8_t idx) {
    if (idx >= ENCODER_COUNT) {
        return 0;
    }
    return s_encoders[idx].event_id;
}
