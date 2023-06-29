#pragma once

#include "../submodules/lua/src/lua.h" // in header
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

// initialize the default crow environment variables & data structures
void l_crowlib_init(lua_State* L);

// destroys user init() function and replaces it with a void fn
void l_crowlib_emptyinit(lua_State* L);

// execute crow.reset() which reverts state of all modules to default
int l_crowlib_crow_reset( lua_State* L );

int l_crowlib_justvolts(lua_State *L);
int l_crowlib_just12(lua_State *L);
int l_crowlib_hztovolts(lua_State *L);
