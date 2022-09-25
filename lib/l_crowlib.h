#pragma once

#include "../submodules/lua/src/lua.h" // in header
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

void l_crowlib_init(lua_State* L);

int l_crowlib_justvolts(lua_State *L);
int l_crowlib_just12(lua_State *L);
int l_crowlib_hztovolts(lua_State *L);
