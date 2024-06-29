#include "lights.h"

#include <stm32f7xx.h>
#include <stdio.h>

typedef struct{
    GPIO_TypeDef* gpio;
    uint32_t pin;
} Pin;

static const Pin ls[12] =
    {{.gpio = GPIOA, .pin = GPIO_PIN_15}
    ,{.gpio = GPIOC, .pin = GPIO_PIN_10}
    ,{.gpio = GPIOC, .pin = GPIO_PIN_11}
    ,{.gpio = GPIOC, .pin = GPIO_PIN_12}
    ,{.gpio = GPIOD, .pin = GPIO_PIN_2}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_3}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_4}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_5}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_6}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_7}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_8}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_9}};

static GPIO_InitTypeDef g;
void lights_init(void){
    // should use an abstracted library rather than this hack
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // GPIO_InitTypeDef g;
    g.Speed = GPIO_SPEED_LOW;

    // this is correct method for leds with current limit
    // g.Pull  = GPIO_NOPULL;
    // g.Mode  = GPIO_MODE_OUTPUT_PP;

    // HACK!
    // here we use pullup as current source, leaving pins as inputs
    g.Pull  = GPIO_NOPULL; // ie. lights off
    g.Mode  = GPIO_MODE_INPUT;

    for(int i=0; i<12; i++){
        Pin lx = ls[i];
        g.Pin   = lx.pin;
        HAL_GPIO_Init(lx.gpio, &g);
    }

    lights_all(0);
}

void lights_all(int state){
    for(int i=0; i<12; i++){
        lights_set(i, state);
    }
}

// this is the only function that calls the LL layer
void lights_set(int ch, int state){
    if(ch<0 || ch>=12){ printf("lights_set(bad_channel)\n\r"); return; } // out of range
    Pin lx = ls[ch];

    // This is the correct method once we add current limiting resistors
    HAL_GPIO_WritePin(lx.gpio, lx.pin, !!state);

    // HACK!!!
    // here we use internal pullups in stm32 as current sources for leds
    // this means the brightness is dim, but at least we don't burn out the leds w/o ilim Rs
    g.Pin = lx.pin;
    g.Pull = !state ? GPIO_NOPULL : GPIO_PULLUP; // pullup controls lit state
    HAL_GPIO_Init(lx.gpio, &g);
}

void lights_xset(int ch){
    lights_all(0);
    lights_set(ch, 1);
}

void lights_range(int min, int max, int state){
    if(min >= max){ printf("lights_range(min must be < max)\n\r"); return; }
    lights_all(0);
    for(int i=min; i<max; i++){
        lights_set(i, !!state);
    }
}
