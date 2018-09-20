#include "lib/lualink.h"

// Lua itself
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "ll/debug_usart.h"

// Lua libs wrapped in C-headers: Note the extra '.h'
#include "lua/crowlib.lua.h"
#include "lua/asl.lua.h"

// Private prototypes
static void Lua_opencrowlibs( lua_State *L );
static void Lua_Test( lua_State *L );


// then call loadstring() over lua_asl which contains the lib as a string
// pointer convention is full address where '/' is replaced with '_' and
// file extension is dropped. eg: 'lua/asl.lua' -> 'lua_asl'

lua_State* L; // global access for 'reset-environment'

// Public functions
void Lua_Init(void)
{
    L = luaL_newstate();  // create a new lua state & save pointer
    luaL_openlibs(L);     // give our lua state access to lua libs

    Lua_opencrowlibs(L);

    //luaL_loadstring(L, lua_asl);

    //int result = lua_pcall(L, 0, 0, 0);
    //if( result ){ fprintf(stderr, "failure\n"); }

    Lua_Test(L);
}

void Lua_DeInit(void)
{
    lua_close(L);
}

static void Lua_Test( lua_State *L )
{
    //float n = 3.0;

    // push a float onto the Lua stack
    //lua_pushnumber(L,n);

    // print to usart
    //char stest[] = "return squared(3)"
    luaL_dostring(L, "return squared(3)" );
    float n = lua_tonumber(L,-1);
    lua_pop(L,1);

    // from lua, call 'squared'
    // squared calls back to C to do the mul
    // lua receives the squared version
    // pop the result back off the stack

    U_PrintF(n); // print to usart

}

// C-library
// to call from Lua
static int L_squared( lua_State *L )
{
    float number = luaL_checknumber(L, 1);
    lua_pushnumber( L, number * number );
    return 1;
}
// array of all the available functions
static const struct luaL_Reg libCrow[]=
    { { "squared"   , L_squared }
    , { NULL        , NULL      }
    };
// make functions available to lua
static void Lua_opencrowlibs( lua_State *L )
{
    // TODO: iterate through luaL_Reg libCrow[]
    lua_pushcfunction(L, L_squared);
    lua_setglobal(L, "squared");
}
