#pragma once

#include <stm32f7xx.h>

#define ADDA_BLOCK_SIZE     32
#define ADDA_DAC_CHAN_COUNT 4
#define ADDA_ADC_CHAN_COUNT 2

typedef struct{
    float    in[ ADDA_ADC_CHAN_COUNT][ADDA_BLOCK_SIZE];
    float    out[ADDA_DAC_CHAN_COUNT][ADDA_BLOCK_SIZE];
    uint16_t size;
} IO_block_t;

uint16_t ADDA_Init( int adc_timer_ix );
void ADDA_Start( void );
void ADDA_BlockProcess( uint32_t* dac_pickle_ptr );

float ADDA_GetADCValue( uint8_t channel );

// __weak definition
// Implement this in library code
// It handles the block-processing of the IO!
IO_block_t* IO_BlockProcess( IO_block_t* b );

// calibration
void CAL_Recalibrate( uint8_t use_defaults );
void CAL_PrintCalibration( void );
