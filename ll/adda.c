#include "adda.h"

#include "debug_pin.h"
#include "ads131.h"
#include "dac8565.h"

#include "../lib/flash.h"              // FLASH_*_t
#include "cal_ll.h"      // CAL_LL_Init(),
#include "../lib/slews.h"             // S_toward()

float _CAL_ADC_GetAverage( uint8_t chan );

typedef enum{ CAL_none
            , CAL_in_shift
            , CAL_in_scale
            , CAL_process_ins
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

#define AVERAGE_COUNT 200
#define AVERAGE_IGNORE 40
#define AVERAGE_USABLE (AVERAGE_COUNT - AVERAGE_IGNORE)
typedef struct {
    CAL_chan_t  adc[2];
    CAL_chan_t  dac[4];
    CAL_stage_t stage;
    int16_t     avg_count;
} CAL_t;

CAL_t cal; // state of the calibration to be used directly, stored & recalled

CAL_stage_t CAL_is_calibrating( void );
IO_block_t* CAL_BlockProcess( IO_block_t* b );

void CAL_ReadFlash( void )
{
    printf("loading calibration data\n");
    if( !Flash_read_calibration( (uint8_t*)(&cal)
                               , sizeof(CAL_chan_t) * (2+4)
                               ) ){
        for( int j=0; j<2; j++ ){
            ADC_CalibrateShift( j, cal.adc[j].shift );
            ADC_CalibrateScalar( j, cal.adc[j].scale );
        }
        for( int j=0; j<4; j++ ){
            DAC_CalibrateOffset( j, cal.dac[j].shift );
            DAC_CalibrateScalar( j, cal.dac[j].scale );
        }
        cal.stage = CAL_none;
    } else {
        printf("calibration readout failed\n");
    }
}

void CAL_WriteFlash( void )
{
    printf("saving calibration data\n");
    if( Flash_write_calibration( (uint8_t*)(&cal)
                               , sizeof(CAL_chan_t) * (2+4)
                               ) ){
        printf("calibration writing failed\n");
    }
}

uint16_t ADDA_Init( void )
{
    ADC_Init( ADDA_BLOCK_SIZE
            , ADDA_ADC_CHAN_COUNT
            );
    DAC_Init( ADDA_BLOCK_SIZE
            , ADDA_DAC_CHAN_COUNT
            );
    CAL_LL_Init();
    cal.stage = CAL_none;
    if( !Flash_is_calibrated() ){ CAL_Recalibrate(); }
    else{                         CAL_ReadFlash(); }
    return ADDA_BLOCK_SIZE;
}

void ADDA_Start( void )
{
    DAC_Start();
}

void ADDA_BlockProcess( uint32_t* dac_pickle_ptr )
{
    IO_block_t b = { .size = ADDA_BLOCK_SIZE };

    ADC_UnpickleBlock( b.in[0]
                     , ADDA_BLOCK_SIZE
                     );
    if( CAL_is_calibrating() ){ CAL_BlockProcess( &b ); }
    else {                      IO_BlockProcess( &b ); }
    DAC_PickleBlock( dac_pickle_ptr
                   , b.out[0]
                   , ADDA_BLOCK_SIZE
                   );
}

float ADDA_GetADCValue( uint8_t channel )
{
    return ADC_GetValue( channel );
}

__weak IO_block_t* IO_BlockProcess( IO_block_t* b )
{
    static float count = 1.0;
    for( uint16_t i=0; i<(b->size); i++ ){
        b->out[0][i] = b->in[0][i];
        b->out[1][i] = b->in[1][i];
        b->out[2][i] = 2.0;
        b->out[3][i] = 3.0;

        count += 0.9; if(count>=3.0){ count -= 6.0; }
    }
    return b;
}

// Calibration process

uint8_t CAL_Step( IO_block_t* b, float* value )
{
    const float input_resistor = 100.0; // 100k
    const float mux_resistor   = 0.33; // 330r
    float resistor_scale = (input_resistor + mux_resistor)
                                    / input_resistor;
    if( cal.avg_count == AVERAGE_COUNT ){
        cal.avg_count--;
        return 1; // FIXME gross logic flow here
    } else if( cal.avg_count <= AVERAGE_USABLE ){
        *value += b->in[0][b->size-1];
    }
    if( !(--cal.avg_count) ){
        *value /= AVERAGE_USABLE;
        *value *= resistor_scale;
        cal.stage++;
        cal.avg_count = AVERAGE_COUNT;
    }
    return 0;
}

IO_block_t* CAL_BlockProcess( IO_block_t* b )
{
    const float vref           = 2.5;
    const float output_ref     = 7.0;
    switch( CAL_is_calibrating() ){
        case CAL_in_shift:
            if( CAL_Step( b, &(cal.adc[0].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_Ground ); } break;
        case CAL_in_scale:
            if( CAL_Step( b, &(cal.adc[0].scale) ) ){
                CAL_LL_ActiveChannel( CAL_LL_2v5 ); } break;

        case CAL_process_ins:
            cal.adc[0].shift = -cal.adc[0].shift;
            ADC_CalibrateShift( 0, cal.adc[0].shift );
            cal.adc[0].scale += cal.adc[0].shift;
            cal.adc[0].scale  = vref / cal.adc[0].scale;
            ADC_CalibrateScalar( 0, cal.adc[0].scale );

            // FIXME: just copying input0 to input1 as we can't fully cal it
            //        could at least take an accurate zero-point
            cal.adc[1].shift = cal.adc[0].shift;
            cal.adc[1].scale = cal.adc[0].scale;
            ADC_CalibrateShift( 1, cal.adc[1].shift );
            ADC_CalibrateScalar( 1, cal.adc[1].scale );

            cal.stage++;
            break;

    // adc is now calibrated
    // proceed to sample each dac

        case CAL_dac0_shift:
            if( CAL_Step( b, &(cal.dac[0].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac0 );
                S_toward( 0, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac0_scale:
            if( CAL_Step( b, &(cal.dac[0].scale) ) ){
                S_toward( 0, output_ref, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_dac1_shift:
            if( CAL_Step( b, &(cal.dac[1].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac1 );
                S_toward( 1, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac1_scale:
            if( CAL_Step( b, &(cal.dac[1].scale) ) ){
                S_toward( 1, output_ref, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_dac2_shift:
            if( CAL_Step( b, &(cal.dac[2].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac2 );
                S_toward( 2, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac2_scale:
            if( CAL_Step( b, &(cal.dac[2].scale) ) ){
                S_toward( 2, output_ref, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_dac3_shift:
            if( CAL_Step( b, &(cal.dac[3].shift) ) ){
                CAL_LL_ActiveChannel( CAL_LL_dac3 );
                S_toward( 3, 0.0, 0.0, SHAPE_Linear, NULL ); } break;
        case CAL_dac3_scale:
            if( CAL_Step( b, &(cal.dac[3].scale) ) ){
                S_toward( 3, output_ref, 0.0, SHAPE_Linear, NULL ); } break;

        case CAL_complete:
            CAL_LL_ActiveChannel( CAL_LL_Ground ); // mux off / gnd input

            for( uint8_t j=0; j<4; j++ ){
                cal.dac[j].shift = -cal.dac[j].shift;
                cal.dac[j].scale = output_ref / (cal.dac[j].scale + cal.dac[j].shift);
                DAC_CalibrateOffset( j, cal.dac[j].shift );
                DAC_CalibrateScalar( j, cal.dac[j].scale );
            }
            CAL_WriteFlash();
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

CAL_stage_t CAL_is_calibrating( void )
{
    return cal.stage;
}

void CAL_Recalibrate( void )
{
    // use default values
    for( int j=0; j<2; j++ ){
        ADC_CalibrateShift( j, 0.0 );
        ADC_CalibrateScalar( j, 1.0 );
    }
    for( int j=0; j<4; j++ ){
        DAC_CalibrateOffset( j, 0.0 );
        DAC_CalibrateScalar( j, 1.0 );
    }
    cal.stage = CAL_in_shift;
    cal.avg_count = AVERAGE_COUNT;
}
