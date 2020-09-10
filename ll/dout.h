#pragma once

#include <stm32f7xx.h>

typedef struct{
    GPIO_TypeDef* gpio;
    uint16_t      pin;
} Dout;

Dout* dout_init( char gpio, int pin );
void dout_set( Dout* o, int state );
void dout_flip( Dout* o );
int dout_get( Dout* o );
