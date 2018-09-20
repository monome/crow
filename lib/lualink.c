#include "lib/lualink.h"

#include <string.h> // strcmp()

// Lua itself
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// Hardware IO
#include "ll/debug_usart.h" // U_Print*()
#include "lib/io.h"         // IO_toward() <- dsp action
#include "lib/caw.h"        // Caw_send_*()
#include "lib/ii.h"         // II_*()

// Lua libs wrapped in C-headers: Note the extra '.h'
#include "lua/bootstrap.lua.h" // MUST LOAD THIS MANUALLY FIRST
#include "lua/crowlib.lua.h"
#include "lua/asl.lua.h"

// Unfortunately the identifiers must be hand converted to this form
// C-preprocessor can't do it all form a single path definition :/
struct lua_lib_locator{
    const char* name;
    const char* addr_of_luacode;
};
const struct lua_lib_locator Lua_libs[] =
    { { "lua_crowlib", lua_crowlib }
    , { "lua_asl"    , lua_asl     }
    , { NULL         , NULL        }
    };

// Basic form of a crow script (in lieu of user-script)
#include "lua/default.lua.h"

// Private prototypes
static void Lua_opencrowlibs( lua_State* L );
static void Lua_loadscript( lua_State* L, const char* script );
static void Lua_crowbegin( lua_State* L );

// Callback prototypes
static void cb_L_toward( int id );

lua_State* L; // global access for 'reset-environment'

// Public functions
void Lua_Init(void)
{
    L = luaL_newstate();  // create a new lua state & save pointer
    luaL_openlibs(L);     // give our lua state access to lua libs
    Lua_opencrowlibs(L);
    Lua_loadscript(L, lua_bootstrap); // redefine dofile() and print()
    Lua_loadscript(L, lua_default);

    Lua_crowbegin(L);
}

void Lua_DeInit(void)
{
    lua_close(L);
}

// C-library
// to call from Lua
//
// go_toward() -- set the dsp engine toward a new target
// send_usb()  -- send a message over usb-uart to host
// send_i2c()  -- send a message over i2c
//
static int L_dofile( lua_State *L )
{
    const char* l_name = luaL_checkstring(L, 1);
    uint8_t i = 0;
    while( Lua_libs[i].addr_of_luacode != NULL ){
        if( !strcmp( l_name, Lua_libs[i].name ) ){ // if the strings match
            if( luaL_dostring( L, Lua_libs[i].addr_of_luacode ) ){
                U_PrintLn("can't load library");
                goto fail;
            }
            return 1; // table is left on the stack as retval
        }
        i++;
    }
    U_PrintLn("can't find library");
fail:
    // failed to find library
    lua_pushnil(L);
    return 1;
}
static int L_debug( lua_State *L )
{
    const char* msg = luaL_checkstring(L, 1);
    U_PrintLn( (char*)msg);
    return 0;
}
static int L_go_toward( lua_State *L )
{
    int id = luaL_checkinteger(L, 1);
    float dest = luaL_checknumber(L, 2);
    float time = luaL_checknumber(L, 3);
    const char* shape = luaL_checkstring(L, 4);

    // now call the function in IO
    U_Print("id\t"); U_PrintU8(id);
    U_Print("dest\t"); U_PrintF(dest);
    U_Print("time\t"); U_PrintF(time);
    U_Print("shape\t"); U_PrintLn( (char*)shape);

    // just put the check*() right in here!
    S_toward( id-1 // C is zero-based
            , dest
            , time*1000.0
            , SHAPE_Linear // Shape_t
            , cb_L_toward
            );
    return 0;
}
static int L_send_usb( lua_State *L )
{
    // pattern match on type: handle values vs strings vs chunk
    const char* msg = luaL_checkstring(L, 1);
    Caw_send_rawtext( (char*) msg );
    return 0;
}
static int L_send_ii( lua_State *L )
{
    // pattern match on broadcast vs query
    uint8_t istate = 4;
    II_broadcast( II_FOLLOW, 1, &istate, 1 );
    return 0;
}
static int L_set_ii_addr( lua_State *L )
{
    // pattern match on broadcast vs query
    uint8_t istate = 4;
    II_broadcast( II_FOLLOW, 1, &istate, 1 );
    return 0;
}
// array of all the available functions
static const struct luaL_Reg libCrow[]=
    { { "c_dofile"   , L_dofile        }
    , { "debug_usart", L_debug         }
    , { "go_toward"  , L_go_toward     }
    , { "send_usb"   , L_send_usb      }
    , { "send_ii"    , L_send_ii       }
    , { "set_ii_addr", L_set_ii_addr   }
    , { NULL         , NULL            }
    };
// make functions available to lua
static void Lua_opencrowlibs( lua_State *L )
{
    // Make C fns available to Lua
    uint8_t fn = 0;
    while( libCrow[fn].func != NULL ){
        lua_pushcfunction( L, libCrow[fn].func );
        lua_setglobal( L, libCrow[fn].name );
        fn++;
    }
}

static void Lua_loadscript( lua_State* L, const char* script ){
    int error = 0;
    if( (error = luaL_loadstring( L, script )) ){
        switch( error ){
            case LUA_ERRSYNTAX: U_PrintLn("can't load script: syntax error"); break;
            case LUA_ERRMEM:    U_PrintLn("can't load script: memory error"); break;
            default: break;
        }
        const char* errmsg = luaL_checkstring(L, -1);
        U_PrintLn( (char*)errmsg );
    }

    if( (error = lua_pcall(L, 0, LUA_MULTRET, 0)) ){
        switch( error ){
            case LUA_ERRRUN: U_PrintLn("can't exec script: runtime error"); break;
            case LUA_ERRMEM: U_PrintLn("can't exec script: memory error"); break;
            case LUA_ERRERR: U_PrintLn("can't exec script: err in err handler"); break;
            default: break;
        }
        const char* errmsg = luaL_checkstring(L, -1);
        U_PrintLn( (char*)errmsg );
    }
}
static void Lua_crowbegin( lua_State* L )
{
    // Call init() function
    // This is all we need to do -> the rest should flow back from Lua
    // The only callback->Lua *not* declared in Lua is a received command over USB
    lua_getglobal(L,"init");
    lua_pcall(L,0,0,0);
}

// Crow callbacks
// TODO: is there anyway *not* to access the global lua_State?
static void cb_L_toward( int id )
{
    lua_getglobal(L, "toward_handler");
    lua_pushinteger(L, id+1); // lua is 1-based
    if( lua_pcall(L, 1, 0, 0) != LUA_OK ){
        U_PrintLn("error running toward_handler");
        U_PrintLn( (char*)lua_tostring(L, -1) );
    }
}
