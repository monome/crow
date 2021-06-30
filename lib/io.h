#pragma once

#include <stm32f7xx.h>
#include <stdbool.h>

void IO_Init( int adc_timer_ix );
void IO_Start( void );

void IO_Process( void );

float IO_GetADC( uint8_t channel );
void IO_SetADCaction( uint8_t channel, const char* mode );

void IO_public_set_view( int chan, bool state );
