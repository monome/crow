#pragma once

// C-ASL
#include "../submodules/lua/src/lua.h"
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

void casl_describe( int index, lua_State* L );
void casl_action( int index, int action );

// dynamic vars
int casl_defdynamic( int index );
void casl_cleardynamics( int index );
void casl_setdynamic( int index, int dynamic_ix, float val );
float casl_getdynamic( int index, int dynamic_ix );
