#include "test.h"

#include "ll/dout.h"
#include "ll/mux.h"

// DUT header io pins are:
// 1 = B8
// 2 = B4
// 3 = C12
// 4 = A8
// 5 = B11
// 6 = B2
// 7 = B0
// 8 = B5
// 9 = B3
// 10= A16
// 11= C6
// 12= B10
// 13= B1


// global objects
static Dout* poweren;
static Mux*  amperes;
static Mux*  adcmux;
static Mux*  dacmux;

// init
void Test_init( void )
{
    // manage power brick
    poweren = dout_init('c',2);
    Test_power( false );

    // monitor power rail consumption
    amperes = mux_init( 'c',3,'c',3,'c',3,'a',0 );
    mux_active( amperes, false );

    // adc mux
    adcmux = mux_init( 'b',8,'b',4,'c',12,'b',0);
    Test_source(0); // disable DUT source

    // dac mux
    dacmux = mux_init( 'a',8,'b',11,'b',2,'a',8);
    // enable pin should not be used!
}

// setters
void Test_power( bool status )
{
    dout_set( poweren, status );
}

void Test_amps( int rail )
{
    if( rail > 0 ){
        mux_select( amperes, rail-1 );
        mux_active( amperes, true );
    } else {
        mux_active( amperes, false );
    }
}

void Test_source( int option ) // ADC read point
{
    if( option > 0 ){
        mux_select( adcmux, option-1 );
        mux_active( adcmux, true );
    } else {
        mux_active( adcmux, false );
    }
}

void Test_dest( int option ) // DAC mux destination
{
    if( option > 0 ){
        mux_select( dacmux, option-1 );
    } else {
        mux_select( dacmux, 3 );
    }
}

// getters
bool Test_is_power( void )
{
    return dout_get( poweren );
}


