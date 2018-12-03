#include "io.h"

#include "main.h"

#include "../ll/cal_ll.h"
#include "../ll/adda.h" // _Init(), _Start(), _GetADCValue(), IO_block_t
#include "../ll/debug_usart.h"
#include "slews.h"

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

// Public Definitions

#include "../../wrDsp/wrOscSine.h"
#include "../../wrLib/wrMath.h"

// currently getting 30 sinewaves at 48kHz
osc_sine_t sinewave[4];

void IO_Init( void )
{
    // TODO: need block_size for anything?
    //uint16_t block_size = ADDA_Init();
    ADDA_Init();

    CAL_LL_Init();
    //IO_Recalibrate();

    //for( uint8_t j=0; j<4; j++ ){
    //    osc_sine_init( &sinewave[j] );
    //    osc_sine_time( &sinewave[j], 0.01*(j*2+1) );
    //}

    S_init();
}

void IO_Start( void )
{
    ADDA_Start();
}

// DSP process
IO_block_t* IO_BlockProcess( IO_block_t* b )
{
    //float adc = b->in[0][0];

    S_step_v( 0
            , b->out[0]
            , b->size
            );
    //for( int j=0; j<SLEW_CHANNELS; j++ ){
    //    S_step_v( j
    //            , b->out[j]
    //            , b->size
    //            );
    //}
    for( int i=0; i<(b->size); i++ ){
        //b->out[0][i] = b->in[0][i];
        b->out[1][i] = 0.0;
        b->out[2][i] = 0.0;
        b->out[3][i] = 0.0;
    }
    return b;
}

float IO_GetADC( uint8_t channel )
{
    return ADDA_GetADCValue( channel );
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
