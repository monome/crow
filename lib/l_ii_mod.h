#pragma once

#include "../submodules/lua/src/lua.h" // lua_State*
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

typedef struct{
    uint8_t cmd;
    const char* name;
} ii_mod_cmds_t;

#define II_MAX_ADDRESSES 8

typedef struct{
    uint8_t     addr;
    const char* name;
    uint8_t     addresses[II_MAX_ADDRESSES]; // leave room for up to 8 so we have a static size
    const ii_mod_cmds_t* commands;
    int         command_count;
} ii_box_t;

void l_ii_mod_preload(lua_State* L);
