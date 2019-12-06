#include "io.h"

#include "stm32f7xx_hal.h"     // HAL_Delay()

#include "../ll/adda.h"        // _Init(), _Start(), _GetADCValue(), IO_block_t
#include "slopes.h"            // S_init(), S_step_v()
#include "detect.h"            // Detect_init(), Detect(), Detect_ix_to_p()
#include "metro.h"

#include "lualink.h"           // L_handle_in_stream (pass this in as ptr?)

#define IN_CHANNELS ADDA_ADC_CHAN_COUNT

void IO_Init( int adc_timer_ix )
{
    // hardware layer
    ADDA_Init(adc_timer_ix);

    // dsp objects
    Detect_init( IN_CHANNELS );
    S_init( SLOPE_CHANNELS );
}

void IO_Start( void )
{
    ADDA_Start();
}

// DSP process
IO_block_t* IO_BlockProcess( IO_block_t* b )
{
    for( int j=0; j<IN_CHANNELS; j++ ){
        Detect( Detect_ix_to_p( j )
              , b->in[j][b->size-1]
              );
    }
    for( int j=0; j<SLOPE_CHANNELS; j++ ){
        S_step_v( j
                , b->out[j]
                , b->size
                );
    }
    return b;
}
float IO_GetADC( uint8_t channel )
{
    return ADDA_GetADCValue( channel );
}
typedef enum{ In_none
            , In_stream
            , In_change
            , In_window
            , In_scale
            , In_quantize
            , In_justintonation
} In_mode_t;
static In_mode_t _parse_mode( const char* mode )
{
    if( *mode == 's' ){
        if( mode[1] == 'c' ){  return In_scale;
        } else {               return In_stream; }
    } else if( *mode == 'c' ){ return In_change;
    } else if( *mode == 'w' ){ return In_window;
    } else if( *mode == 'q' ){ return In_quantize;
    } else if( *mode == 'j' ){ return In_justintonation;
    } else {                   return In_none;
    }
}
void IO_SetADCaction( uint8_t channel, const char* mode )
{
    switch( _parse_mode(mode) ){
        case In_none:
            break;
        case In_stream:
            break;
        case In_change:
            break;
        case In_window:
            break;
        case In_scale:
            break;
        case In_quantize:
            break;
        case In_justintonation:
            break;
        default: break;
    }
    // set the appropriate fn to be called in ADC dsp loop
}
