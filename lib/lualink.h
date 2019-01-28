#pragma once

#include <stdint.h>

// Lua itself
#include "../submodules/lua/src/lua.h" // lua_State*

typedef void (*ErrorHandler_t)(char* error_message);
struct lua_lib_locator{ const char* name; const char* addr_of_luacode; };

lua_State* Lua_Init(void);
void Lua_DeInit(void);

void Lua_crowbegin(void);
uint8_t Lua_eval( lua_State*     L
                , const char*    script
                , size_t         script_len
                , ErrorHandler_t errfn
                );
void Lua_load_default_script( void );

// Callback declarations
extern void L_handle_toward( int id );
extern void L_handle_metro( const int id, const int stage);
extern void L_handle_in_stream( int id, float value );
extern void L_handle_change( int id, float state );
