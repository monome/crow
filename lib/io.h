#pragma once

#include <stm32f7xx.h>

void IO_Init( void );

void IO_Process( void );
void IO_Recalibrate( void );

// these 2 will need some intermediate layer for block-processing?
void IO_Set( uint8_t channel, float volts );
float IO_Get( uint8_t channel );
