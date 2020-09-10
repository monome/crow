#pragma once

#include <stdbool.h>

#include "dout.h"

typedef struct{
    Dout* pin[3];
    Dout* en;
} Mux;

Mux* mux_init( char port0 , int pin0
             , char port1 , int pin1
             , char port2 , int pin2
             , char portEn, int pinEn );
void mux_select( Mux* m, int channel );
void mux_active( Mux* m, bool is_active );
