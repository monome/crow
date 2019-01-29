#pragma once

#include "lib/lualink.h" // struct lua_lib_locator

#include "build/ii_jf.lua.h"
#include "build/ii_wslash.lua.h"

const struct lua_lib_locator Lua_ii_libs[] = {
	{ "build_ii_jf", build_ii_jf },
	{ "build_ii_wslash", build_ii_wslash },
	{ NULL, NULL } };
