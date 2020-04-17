#pragma once

#include <stm32f7xx.h>
#include "slopes.h" // Slope_t

#define IN_CHANNELS    ADDA_ADC_CHAN_COUNT
#define OUT_CHANNELS   ADDA_DAC_CHAN_COUNT
#define SLOPE_CHANNELS 4

void IO_Init( void );
void IO_Start( void );

void IO_Process( void );

Slope_t* IO_Slope_ptr_from_ix( int ix );
float IO_GetADC( uint8_t channel );
float IO_GetDAC( int channel );
void IO_SetADCaction( uint8_t channel, const char* mode );
