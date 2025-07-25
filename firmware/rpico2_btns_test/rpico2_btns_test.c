#include <stdio.h>
#include "pico/stdlib.h"
#include "rpico2_btns_test.h"

int main()
{
    stdio_init_all();
    hardware_init();
    
    printf("Hello, world!\n");
    while (true) {
        if (buttons[0].pressed | buttons[1].pressed | buttons[2].pressed)
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        else
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        check_debounced_buttons();
        sleep_ms(100);
    }
}

void hardware_init(void)
{

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  gpio_set_irq_enabled_with_callback(buttons[0].gpio, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(buttons[i].gpio);
        gpio_set_dir(buttons[i].gpio, GPIO_IN);
        gpio_pull_up(buttons[i].gpio);
        gpio_set_irq_enabled(buttons[i].gpio, GPIO_IRQ_EDGE_FALL, true);
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (buttons[i].gpio == gpio && buttons[i].debounced) {
            bool state = (events & GPIO_IRQ_EDGE_FALL) != 0;
            buttons[i].pressed = state;
            buttons[i].last_time_us = time_us_32();
            buttons[i].debounced = false;
            gpio_set_irq_enabled(gpio, events, false); // Bloquear hasta confirmar
            break;
        }
    }
}


void check_debounced_buttons(void) {
    uint32_t now = time_us_32();

    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (!buttons[i].debounced && (now - buttons[i].last_time_us > DEBOUNCE_DELAY_US)) {
          if (gpio_get(buttons[i].gpio)) {
            gpio_set_irq_enabled(buttons[i].gpio, GPIO_IRQ_EDGE_FALL, true);
            buttons[i].pressed = false;
          } else {
            gpio_set_irq_enabled(buttons[i].gpio, GPIO_IRQ_EDGE_RISE, true);
          }
          buttons[i].debounced = true;
        }
    }
}
