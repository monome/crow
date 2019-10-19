#pragma once

#include <stdint.h>

// Lua itself
#include "../submodules/lua/src/lua.h" // lua_State*

typedef void (*ErrorHandler_t)(char* error_message);
struct lua_lib_locator{ const char* name; const char* addr_of_luacode; };

lua_State* Lua_Init(void);
void Lua_Reset(void);
void Lua_DeInit(void);

void Lua_crowbegin(void);
uint8_t Lua_eval( lua_State*     L
                , const char*    script
                , size_t         script_len
                , ErrorHandler_t errfn
                );
void Lua_load_default_script( void );
void Lua_pause_inputs( void );
void Lua_resume_inputs( void );

// Event enqueue wrappers
extern void L_queue_toward( int id );
extern void L_queue_metro( int id, int state );
extern void L_queue_in_stream( int id );
extern void L_queue_change( int id, float state );
extern void L_queue_midi( uint8_t* data );

// Callback declarations
extern void L_handle_toward( int id );
extern void L_handle_metro( const int id, const int stage);
extern void L_handle_in_stream( int id, float value );
extern void L_handle_change( int id, float state );
extern void L_handle_ii_leadRx( uint8_t address, uint8_t cmd, float data );
extern void L_handle_ii_followRx( uint8_t cmd, int args, float* data );
extern float L_handle_ii_followRxTx( uint8_t cmd, int args, float* data );
extern void L_handle_midi( uint8_t* data );
