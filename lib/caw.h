#pragma once

#include <stm32f7xx.h>

typedef enum{ C_none
            , C_repl
            , C_boot
            , C_flashstart
            , C_flashend
} C_cmd_t;

uint8_t Caw_Init( void );

void Caw_send_raw( uint8_t* buf, uint32_t len );
void Caw_send_luachunk( char* text );
void Caw_send_luaerror( char* error_msg );
void Caw_send_value( uint8_t type, float value ); // enum the type

C_cmd_t Caw_try_receive( void );
char* Caw_get_read( void );
uint32_t Caw_get_read_len( void );
