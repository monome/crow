#include "io.h"

#include "main.h"

#include "../ll/cal_ll.h"
#include "../ll/dac8565.h"
#include "../ll/ads131.h"
#include "../ll/debug_usart.h"

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
void IO_Init( void )
{
    uint32_t block_size = DAC_Init();
    IO_Set( DAC_ALL_CHANNELS, 0.0 );
    DAC_Update();

    ADC_Init();

    CAL_LL_Init();
    //IO_Recalibrate();
}

void IO_Start( void )
{
    DAC_Start();
}

// This needs to be reconfigured as a callback from the DAC driver
// Only trigger the 'process' every 4th cycle
// 1: average ADC since last sample
void IO_Process( void )
{
    DAC_Update();
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
    U_PrintU16(ADC_GetU16(0));
    HAL_Delay(del);

    // ADC calibration against reference
    CAL_LL_ActiveChannel( CAL_LL_Reference );
    HAL_Delay(del);
    U_PrintU16(ADC_GetU16(0));
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
#define DAC_ZERO_VOLTS      ((uint16_t)(((uint32_t)0xFFFF * 2)/3))
//#define DAC_ZERO_VOLTS 0
#define DAC_V_TO_U16        ((float)(65535.0 / 15.0))
void IO_Set( uint8_t channel, float volts )
{
    // TODO: apply calibration first
    // TODO: roll calibration & scaling into one for efficiency
    DAC_SetU16( channel
              , (uint16_t)( DAC_ZERO_VOLTS - (int16_t)(volts * DAC_V_TO_U16) )
              );
}
float IO_Get( uint8_t channel )
{
    // TODO: apply calibration first
    // TODO: roll calibration & scaling into one for efficiency
    return ((float)ADC_GetU16(channel) / DAC_V_TO_U16 - 5.0);
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
        DAC_Update();
        // TODO: Wait for DAC DMA setting time
        // TODO: Wait for DAC settling time
        //float avg = _CAL_ADC_GetAverage(0); // always use 1st input
    } while(0); // TODO: Test for zero point
    // TODO: Repeat for scaling
}

float _CAL_ADC_GetAverage( uint8_t chan )
{
    // TODO: average a number of calls (blocking)
    return IO_Get( chan );
}
