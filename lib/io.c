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

typedef enum{ CAL_none
            , CAL_in_shift
            , CAL_in_scale
            , CAL_dac0_shift
            , CAL_dac0_scale
            , CAL_dac1_shift
            , CAL_dac1_scale
            , CAL_dac2_shift
            , CAL_dac2_scale
            , CAL_dac3_shift
            , CAL_dac3_scale
            , CAL_complete
} CAL_stage_t;

typedef struct {
    float shift;
    float scale;
} CAL_chan_t;

#define AVERAGE_COUNT 100
#define AVERAGE_IGNORE 20
#define AVERAGE_USABLE (AVERAGE_COUNT - AVERAGE_IGNORE)
typedef struct {
    int16_t     avg_count;
    CAL_stage_t stage;
    CAL_chan_t  adc[2];
    CAL_chan_t  dac[4];
} CAL_t;

CAL_t cal; // state of the calibration to be used directly, stored & recalled

CAL_stage_t IO_calibrating( void );
IO_block_t* IO_CalibrationBlockProcess( IO_block_t* b );

void IO_Init( void )
{
    ADDA_Init();

    CAL_LL_Init();
    if( !Flash_is_calibrated() ){ IO_Recalibrate(); }
    else{} // TODO read flash calibration into cal (or at least zero out)

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
    if( IO_calibrating() ){ return IO_CalibrationBlockProcess( b ); }

    // main functionality!
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

uint8_t CAL_Step( IO_block_t* b, float* value )
{
    if( cal.avg_count == AVERAGE_COUNT ){
        cal.avg_count--;
        return 1; // FIXME gross logic flow here
    } else if( cal.avg_count <= AVERAGE_USABLE ){
        *value += b->in[0][b->size-1];
    }
    if( !(--cal.avg_count) ){
        *value /= AVERAGE_USABLE;
        cal.stage++;
        cal.avg_count = AVERAGE_COUNT;
    }
    return 0;
}

IO_block_t* IO_CalibrationBlockProcess( IO_block_t* b )
{
    switch( IO_calibrating() ){
        case CAL_in_shift:
            if( CAL_Step( b, &(cal.adc[0].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_Ground ); } break;
        case CAL_in_scale:
            if( CAL_Step( b, &(cal.adc[0].scale) ) ){
                CAL_LL_ActiveChannel( CAL_LL_2v5 ); } break;

        case CAL_dac0_shift:
            if( CAL_Step( b, &(cal.dac[0].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac0 );
                S_toward( 0, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac0_scale:
            if( CAL_Step( b, &(cal.dac[0].scale) ) ){
                S_toward( 0, 7.0, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_dac1_shift:
            if( CAL_Step( b, &(cal.dac[1].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac1 );
                S_toward( 1, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac1_scale:
            if( CAL_Step( b, &(cal.dac[1].scale) ) ){
                S_toward( 1, 7.0, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_dac2_shift:
            if( CAL_Step( b, &(cal.dac[2].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac2 );
                S_toward( 2, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac2_scale:
            if( CAL_Step( b, &(cal.dac[2].scale) ) ){
                S_toward( 2, 7.0, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_dac3_shift:
            if( CAL_Step( b, &(cal.dac[3].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac3 );
                S_toward( 3, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac3_scale:
            if( CAL_Step( b, &(cal.dac[3].scale) ) ){
                S_toward( 3, 7.0, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_complete:
            // process!
            U_PrintF(cal.dac[0].shift);
            U_PrintF(cal.dac[0].scale);
            U_PrintLn("done calibration");
            cal.stage = CAL_none;
            break;
        default: break;
    }

    // run slopes to set output levels
    for( int j=0; j<SLEW_CHANNELS; j++ ){
        S_step_v( j
                , b->out[j]
                , b->size
                );
    }
    return b;
}

CAL_stage_t IO_calibrating( void )
{
    return cal.stage;
}

void IO_Recalibrate( void )
{
    cal.stage = CAL_in_shift;
    cal.avg_count = AVERAGE_COUNT;
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
