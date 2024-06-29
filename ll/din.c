#include "din.h"

#include <stm32f7xx.h>
#include <stdio.h>

typedef struct{
    GPIO_TypeDef* gpio;
    uint32_t pin;
} Pin;

static const Pin ds[3] =
    {{.gpio = GPIOB, .pin = GPIO_PIN_12}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_13}
    ,{.gpio = GPIOB, .pin = GPIO_PIN_14}};

static GPIO_InitTypeDef g;

void din_init(void){
    __HAL_RCC_GPIOB_CLK_ENABLE();

    g.Speed = GPIO_SPEED_HIGH;
    g.Pull  = GPIO_NOPULL;
    g.Mode  = GPIO_MODE_INPUT;

    for(int i=0; i<3; i++){
        Pin dx = ds[i];
        g.Pin = dx.pin;
        HAL_GPIO_Init(dx.gpio, &g);
    }
}

int din_get(DINx ch){
    if(ch<0 || ch>=3){ printf("din_get(bad_channel)\n\r"); return -1; } // out of range
    Pin dx = ds[ch];
    return (int)!HAL_GPIO_ReadPin(dx.gpio, dx.pin); // pins are inverted (active low)
}
