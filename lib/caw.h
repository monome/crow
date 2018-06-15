#pragma once

#include <stm32f7xx.h>

void Caw_send_rawtext( char* text );
void Caw_send_luachunk( char* text );
void Caw_send_value( uint8_t type, float value ); // enum the type

uint8_t Caw_try_receive( void );

// Weak callbacks. Define in top-level program
__weak void Caw_receive_rawtext_callback( uint8_t* buf, uint32_t len );
