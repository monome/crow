#include "status_led.h"

#include <stdbool.h>

// LED on GPIO A15

static GPIO_InitTypeDef GPIO_IS;
static LED_SPEED fast_blink;
static bool do_flip = false;

void status_led_init(void){
    // activate peripheral clock
    __HAL_RCC_GPIOA_CLK_ENABLE();
    // set constant params of gpio init struct
    GPIO_IS.Pin   = GPIO_PIN_15;
    GPIO_IS.Pull  = GPIO_NOPULL;
    GPIO_IS.Speed = GPIO_SPEED_LOW;
    GPIO_IS.Mode  = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_IS);
    status_led_set(1); // default to ON
}

// ONLY CALL WHEN time_now HAS CHANGED
// call this at a regular interval (designed for 1ms interval)
uint32_t next_flip = 0;
void status_led_tick(uint32_t time_now){
    if(time_now > next_flip // time to animate timer
    || (do_flip && fast_blink)){ // xor event & USB connected
        // status_led_xor();
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
        next_flip = time_now + (fast_blink ? 2000 : 500);
        do_flip = false;
    }
}

void status_led_fast(LED_SPEED is_fast){
    fast_blink = is_fast;
}

void status_led_set(uint8_t is_on){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, is_on);
}

void status_led_xor(void){
    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
    do_flip = true;
}
