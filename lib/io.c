#include "io.h"

#include "stm32f7xx_hal.h"     // HAL_Delay()

#include "../ll/adda.h"        // _Init(), _Start(), _GetADCValue(), IO_block_t
#include "detect.h"            // Detect_init(), Detect(), Detect_ix_to_p()
#include "metro.h"

#include "lualink.h"           // L_handle_in_stream (pass this in as ptr?)


/////////////////////////////////////
// Global vars

Slope_t* slopes[SLOPE_CHANNELS];
float    last_out[OUT_CHANNELS];


////////////////////////////////////
// Public defns

void IO_Init( void )
{
    // hardware layer
    ADDA_Init();

    // dsp objects
    Detect_init( IN_CHANNELS );
    for( int i=0; i<SLOPE_CHANNELS; i++ ){
        slopes[i] = S_init(); // statically allocate max ASL slopes
    }
}

void IO_Start( void )
{
    ADDA_Start();
}


//////////////////////////////////////
// DSP process

IO_block_t* IO_BlockProcess( IO_block_t* b )
{
    for( int j=0; j<IN_CHANNELS; j++ ){
        Detect( Detect_ix_to_p( j )
              , b->in[j][b->size-1]
              );
    }
    //float tmp[SLOPE_CHANNELS-OUT_CHANNELS][b->size];
    //for( int j=OUT_CHANNELS; j<SLOPE_CHANNELS; j++ ){ // modulation channels
    //    Slope_v( slopes[j]
    //           , tmp[j-OUT_CHANNELS]
    //           , b->size
    //           );
    //}
    for( int j=0; j<OUT_CHANNELS; j++ ){ // physical outputs
        Slope_v( slopes[j]
               , b->out[j]
               , b->size
               );
        last_out[j] = b->out[j][b->size-1]; // save output value for query
    }
    return b;
}


////////////////////////////////////
// Configuration

Slope_t* IO_Slope_ptr_from_ix( int ix )
{
    if( ix < 0 || ix > SLOPE_CHANNELS ){ return NULL; }
    return slopes[ix];
}

float IO_GetADC( uint8_t channel )
{
    return ADDA_GetADCValue( channel );
}

float IO_GetDAC( int channel )
{
    if( channel < 0 || channel > OUT_CHANNELS ){ return 0.0; }
    return last_out[channel];
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
