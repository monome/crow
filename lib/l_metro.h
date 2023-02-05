#pragma once

#include <stdbool.h>

#include "../submodules/lua/src/lua.h" // in header
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

// initialize the module itself
int l_metrolib_init( lua_State *L );

// module actions
// stops then re-initializes all metros to defaults
int l_metro_free_all( lua_State *L );
// allocate a metro with config parameters
int l_metro_init( lua_State *L );


// class-local actions
int l_metro_start( lua_State *L );
int l_metro_stop( lua_State *L );

// metamethods
int l_metro__index( lua_State *L );
int l_metro__newindex( lua_State *L );

// event handler
extern void L_queue_metro( int id, int state );
