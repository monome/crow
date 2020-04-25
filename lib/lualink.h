#pragma once

#include <stdint.h>

// Lua itself
#include "../submodules/lua/src/lua.h" // lua_State*

typedef void (*ErrorHandler_t)(char* error_message);
struct lua_lib_locator{ const char* name; const char* addr_of_luacode; };

extern volatile int CPU_count; // count from main.c

lua_State* Lua_Init(void);
lua_State* Lua_Reset( void );
void Lua_DeInit(void);

void Lua_crowbegin( void );
uint8_t Lua_eval( lua_State*     L
                , const char*    script
                , size_t         script_len
                , const char*    chunkname
                );
void Lua_load_default_script( void );

// Event enqueue wrappers
extern void L_queue_toward( int id );
extern void L_queue_metro( int id, int state );
extern void L_queue_in_stream( int id );
extern void L_queue_change( int id, float state );
extern void L_queue_midi( uint8_t* data );
extern void L_queue_window( int id, float window );
extern void L_queue_ii_leadRx( uint8_t address, uint8_t cmd, float data );
extern void L_queue_ii_followRx( void );

// Callback declarations
extern float L_handle_ii_followRxTx( uint8_t cmd, int args, float* data );
extern void L_handle_ii_followRx_cont( uint8_t cmd, int args, float* data );
