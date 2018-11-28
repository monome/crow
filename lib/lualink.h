#pragma once

#include <stdint.h>

typedef void (*ErrorHandler_t)(char* error_message);

void Lua_Init(void);
void Lua_DeInit(void);

void Lua_crowbegin(void);
void Lua_repl( char* buf, uint32_t len, ErrorHandler_t errfn );
