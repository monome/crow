#include "mux.h"

#include <stdio.h>
#include <stdlib.h>

Mux* mux_init( char port0 , int pin0
             , char port1 , int pin1
             , char port2 , int pin2
             , char portEn, int pinEn )
{
    Mux* m = malloc( sizeof( Mux ));
    if( !m ){ printf("mux malloc!\n"); return NULL; }

    m->pin[0] = dout_init( port0,  pin0 );
    m->pin[1] = dout_init( port1,  pin1 );
    m->pin[2] = dout_init( port2,  pin2 );
    m->en     = dout_init( portEn, pinEn );

    mux_select( m, 0 );
    mux_active( m, true );

    return m;
}

void mux_select( Mux* m, int channel )
{
    dout_set( m->pin[0], (channel & 0x1) );
    dout_set( m->pin[1], (channel & 0x2) >> 1 );
    dout_set( m->pin[2], (channel & 0x4) >> 2 );
}

void mux_active( Mux* m, bool is_active )
{
    dout_set( m->en, !is_active ); // hw is active=LOW
}
