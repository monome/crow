#include "adda.h"

#include <string.h>
#include <stdio.h>

#include "debug_pin.h"
#include "ads131.h"
#include "dac8565.h"

#include "../lib/flash.h"              // FLASH_*_t
#include "cal_ll.h"      // CAL_LL_Init(),
#include "../lib/slopes.h"             // S_toward()
#include "../lib/caw.h" // Caw_send_raw

typedef struct {
    float shift;
    float scale;
} CAL_chan_t;

typedef struct {
    CAL_chan_t  adc[2];
    CAL_chan_t  dac[4];
} CAL_t;

static CAL_t cal;
static void CAL_ReadFlash( void );


uint16_t ADDA_Init( int adc_timer_ix )
{
    ADC_Init( ADDA_BLOCK_SIZE
            , ADDA_ADC_CHAN_COUNT
            , adc_timer_ix
            );
    DAC_Init( ADDA_BLOCK_SIZE
            , ADDA_DAC_CHAN_COUNT
            );
    CAL_LL_Init();
    CAL_ReadFlash();
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
    IO_BlockProcess( &b );
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
    for( uint16_t i=0; i<(b->size); i++ ){
        b->out[0][i] = b->in[0][i];
        b->out[1][i] = b->in[1][i];
        b->out[2][i] = 2.0;
        b->out[3][i] = 3.0;
    }
    return b;
}


////////////////////////////////////////////
// Calibration

void CAL_WriteFlash( void )
{
    if( Flash_write_calibration( (uint8_t*)(&cal)
                               , sizeof(CAL_chan_t) * (2+4)
                               ) ){
        char msg[] = "Error saving Cal data to flash!\n";
        Caw_send_raw( (uint8_t*)msg, strlen(msg) );
        printf(msg);
    } else {
        char msg[] = "Calibration ok!\n\r";
        Caw_send_raw( (uint8_t*)msg, strlen(msg) );
        printf(msg);
    }
}

void CAL_Set( int chan, CAL_Param_t param, float val )
{
    if(chan >= 1 && chan <= 2){ // adc
        if(param == CAL_Offset){
            cal.adc[chan-1].shift = val;
            ADC_CalibrateShift(chan-1, val);
        } else {
            cal.adc[chan-1].scale = val;
            ADC_CalibrateScalar(chan-1, val);
        }
    } else if( chan >= 3 && chan <= 6){ // dac
        if(param == CAL_Offset){
            cal.dac[chan-3].shift = val;
            DAC_CalibrateOffset(chan-3, val);
        } else {
            cal.dac[chan-3].scale = val;
            DAC_CalibrateScalar(chan-3, val);
        }
    }
}

float CAL_Get( int chan, CAL_Param_t param )
{
    if(chan >= 1 && chan <= 2){ // adc
        if(param == CAL_Offset){
            return cal.adc[chan-1].shift;
        } else {
            return cal.adc[chan-1].scale;
        }
    } else if( chan >= 3 && chan <= 6){ //dac
        if(param == CAL_Offset){
            return cal.dac[chan-3].shift;
        } else {
            return cal.dac[chan-3].scale;
        }
    } else { return 0.0; }
}

static void CAL_Defaults( void )
{
    for( int j=1; j<7; j++ ){
        CAL_Set(j, CAL_Offset, 0.0);
        CAL_Set(j, CAL_Scale, 1.0);
    }
}

static void CAL_ReadFlash( void )
{
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
    } else {
        printf("not yet calibrated\n");
        CAL_Defaults();
    }
}
