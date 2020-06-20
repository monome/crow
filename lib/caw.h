#pragma once

#include <stm32f7xx.h>

typedef enum{ C_none
            , C_repl
            , C_boot
            , C_startupload
            , C_endupload
            , C_flashupload
            , C_flashclear
            , C_restart
            , C_print
            , C_version
            , C_identity
            , C_killlua
            , C_loadFirst
} C_cmd_t;

void Caw_Init( int timer_index );
void Caw_DeInit( void );

void Caw_send_raw( uint8_t* buf, uint32_t len );
void Caw_send_luachunk( char* text );
void Caw_send_luaerror( char* error_msg );
void Caw_send_value( uint8_t type, float value ); // enum the type

C_cmd_t Caw_try_receive( void );
char* Caw_get_read( void );
uint32_t Caw_get_read_len( void );
