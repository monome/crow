#pragma once

#include <stm32f7xx.h>

typedef enum{ LED_SLOW = 0
            , LED_FAST = 1
} LED_SPEED;

void status_led_init(void);

// ONLY CALL WHEN time_now HAS CHANGED
// call this at a regular interval (designed for 1ms interval)
void status_led_tick(uint32_t time_now);

void status_led_fast(LED_SPEED is_fast);
void status_led_set(uint8_t is_on);
void status_led_xor(void);
