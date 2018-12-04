#pragma once

#include <stdint.h>

typedef void (*ErrorHandler_t)(char* error_message);

void Lua_Init(void);
void Lua_DeInit(void);

void Lua_crowbegin(void);
void Lua_repl( char* buf, uint32_t len, ErrorHandler_t errfn );
void Lua_receive_script( char* buf, uint32_t len, ErrorHandler_t errfn );
void Lua_load_new_script( ErrorHandler_t errfn );

// Callback declarations
extern void L_handle_toward( int id );
extern void L_handle_metro( const int id, const int stage);
