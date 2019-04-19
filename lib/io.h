#pragma once

#include <stm32f7xx.h>

void IO_Init( void );
void IO_Start( void );

void IO_Process( void );

float IO_GetADC( uint8_t channel );
void IO_SetADCaction( uint8_t channel, const char* mode );
