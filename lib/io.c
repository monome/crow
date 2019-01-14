#include "io.h"

#include "flash.h"              // FLASH_*_t
#include "stm32f7xx_hal.h"     // HAL_Delay()

#include "../ll/cal_ll.h"      // CAL_LL_Init(),
#include "../ll/adda.h"        // _Init(), _Start(), _GetADCValue(), IO_block_t
#include "slews.h"             // S_init(), S_step_v()
#include "detect.h"            // Detect_init(), Detect(), Detect_ix_to_p()
#include "metro.h"

#include "lualink.h"           // L_handle_in_stream (pass this in as ptr?)

#include "../ll/debug_usart.h" // U_Print*()

#define IN_CHANNELS ADDA_ADC_CHAN_COUNT

// Private Declarations
void _CAL_DAC( uint8_t chan );
float _CAL_ADC_GetAverage( uint8_t chan );

typedef struct {
    float shift;
    float scale;
} CAL_chan_t;

typedef struct {
    CAL_chan_t adc;
    CAL_chan_t dac[4];
} CAL_t;

CAL_t cal;

void IO_Init( void )
{
    ADDA_Init();

    //CAL_LL_Init();
    //IO_Recalibrate();

    Detect_init( IN_CHANNELS );
    S_init();
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
    for( int j=0; j<SLEW_CHANNELS; j++ ){
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

void IO_handle_timer( uint8_t channel )
{
    L_handle_in_stream( channel, IO_GetADC(channel) );
}

void IO_Recalibrate( void )
{
    // Only calibrate if no current data saved
    // Can force recalibration in program with:
        // save->status = FLASH_Status_Init;
        // before calling CAL_Process();
    /*if( save->status == FLASH_Status_Saved ){
        // Copy saved data into local struct
        return save;
    }*/

    uint32_t del = 1;
    HAL_Delay(del);
    //U_PrintU16(ADC_GetU16(0));
    HAL_Delay(del);

    // ADC calibration against reference
    CAL_LL_ActiveChannel( CAL_LL_Reference );
    HAL_Delay(del);
    //U_PrintU16(ADC_GetU16(0));
    HAL_Delay(del);


    // DAC calibration sequence
    for( uint8_t i=0; i<4; i++ ){
        _CAL_DAC(i);
    }

    // Deactivate Calibration
    CAL_LL_ActiveChannel( CAL_LL_Disable );

    // TODO: write data into save

    //return save;
}
// 0xAAAA
//#define DAC_ZERO_VOLTS 0
void IO_Set( uint8_t channel, float volts )
{
    // TODO: apply calibration first
    // TODO: roll calibration & scaling into one for efficiency
    /*DAC_SetU16( channel
              , (uint16_t)( DAC_ZERO_VOLTS - (int16_t)(volts * DAC_V_TO_U16) )
              );
              */
}

// Private definitions
const CAL_LL_Channel_t cal_chan[4] = { CAL_LL_0
                                     , CAL_LL_1
                                     , CAL_LL_2
                                     , CAL_LL_3
                                     };
void _CAL_DAC( uint8_t chan )
{
    CAL_LL_ActiveChannel( cal_chan[chan] );
    // TODO: Set calibration data to defaults
    do{
        // TODO: shift calibration toward destination
        IO_Set( chan, 0.0 );
        //DAC_Update();
        // TODO: Wait for DAC DMA setting time
        // TODO: Wait for DAC settling time
        //float avg = _CAL_ADC_GetAverage(0); // always use 1st input
    } while(0); // TODO: Test for zero point
    // TODO: Repeat for scaling
}

float _CAL_ADC_GetAverage( uint8_t chan )
{
    // TODO: average a number of calls (blocking)
    return IO_GetADC( chan );
}
