#pragma once

#include <stm32f7xx.h>

// lua_State*
#include "../submodules/lua/src/lua.h"
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

#include "lualink.h" // ErrorHandler_t, Lua_eval(), Lua_load_default_script()

// types
typedef enum{ REPL_normal
            , REPL_reception
} L_repl_mode;

void REPL_init( lua_State* lua );
void REPL_mode( L_repl_mode mode );
void REPL_eval( char* buf, uint32_t len, ErrorHandler_t errfn );
void REPL_print_script( void );
