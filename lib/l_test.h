#pragma once

#include "../submodules/lua/src/lua.h" // in header
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

// exposes test platform controls to lua environment
void l_test_preload(lua_State* L);
