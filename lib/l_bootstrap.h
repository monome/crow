#pragma once

#include "../submodules/lua/src/lua.h" // in header
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

void l_bootstrap_init(lua_State* L);
int l_bootstrap_dofile(lua_State* L);
