#include "lib/lualink.h"


#include <string.h> // strcmp(), strlen()
#include <stdlib.h> // malloc(), free()

// Lua itself
#include "../submodules/lua/src/lua.h"
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

// Hardware IO
#include "ll/debug_usart.h" // U_Print*()
#include "lib/slews.h"      // S_toward
#include "lib/caw.h"        // Caw_send_*()
#include "lib/ii.h"         // II_*()
#include "lib/bootloader.h" // bootloader_enter()
#include "lib/metro.h"      // metro_start() metro_stop() metro_set_time()
#include "lib/io.h"         // IO_GetADC()
#include "lib/flash.h"      // Flash_*()

// Lua libs wrapped in C-headers: Note the extra '.h'
#include "lua/bootstrap.lua.h" // MUST LOAD THIS MANUALLY FIRST
#include "lua/crowlib.lua.h"
#include "lua/asl.lua.h"
#include "lua/asllib.lua.h"
#include "lua/metro.lua.h"

struct lua_lib_locator{ const char* name; const char* addr_of_luacode; };
const struct lua_lib_locator Lua_libs[] =
    { { "lua_crowlib", lua_crowlib }
    , { "lua_asl"    , lua_asl     }
    , { "lua_asllib" , lua_asllib  }
    , { "lua_metro"  , lua_metro   }
    , { NULL         , NULL        }
    };

// Basic crow script
#include "lua/default.lua.h"

// Private prototypes
static void Lua_linkctolua( lua_State* L );
static uint8_t Lua_eval( lua_State* L, const char* script, ErrorHandler_t errfn );

lua_State* L; // global access for 'reset-environment'

// repl / script load stuff (TODO needs its own file)
char*   new_script;
char* p_new_script;
static void Lua_new_script_buffer( void );
L_repl_mode repl_mode = REPL_normal;

// Public functions
void Lua_Init(void)
{
    L = luaL_newstate();
    luaL_openlibs(L);                      // lua std lib
    Lua_linkctolua(L);                     // lua can access declared c fns
    Lua_eval(L, lua_bootstrap, U_PrintLn); // redefine dofile(), print(), load crowlib

    // TODO this will first check for a user script
    // fallback if syntax error
    if( Flash_is_user_script() ){
        U_PrintLn("load user script");
        // TODO load user script
        Lua_new_script_buffer();
        Flash_read_user_script( new_script );
        // now loadstring & pcall new_script
        if( Lua_eval( L, new_script, Caw_send_luaerror ) ){
            U_PrintLn("failed to load user script");
        }
        free(new_script);
    } else {
        U_PrintLn("load default script");
        Lua_eval(L, lua_default, U_PrintLn);   // run default script
    }
}

void Lua_DeInit(void)
{
    lua_close(L);
}
void check_ram_usage( void )
{
//    int s; // stack allocation
//    int* h = malloc(sizeof(int)); // heap allocation
//    U_Print("ram left "); U_PrintU32( (int)&s - (int)h );
//    //U_Print("stack "); U_PrintU32( (int)&s );
//    //U_Print("heap  "); U_PrintU32( (int)h );
//    free(h);
}

// C-fns accessible to lua

// NB these static functions are prefixed  with '_'
// to avoid shadowing similar-named extern functions in other modules
// and also to distinguish from extern 'L_' functions.
static int _dofile( lua_State *L )
{
    const char* l_name = luaL_checkstring(L, 1);
    lua_pop( L, 1 );
    uint8_t i = 0;
    while( Lua_libs[i].addr_of_luacode != NULL ){
        if( !strcmp( l_name, Lua_libs[i].name ) ){ // if the strings match
            if( luaL_dostring( L, Lua_libs[i].addr_of_luacode ) ){
                U_Print("can't load library: ");
                U_PrintLn( (char*)Lua_libs[i].name );
                goto fail;
            }
            //check_ram_usage();
            return 1; // table is left on the stack as retval
        }
        i++;
    }
    U_Print("can't find library: ");
    U_PrintLn( (char*)l_name );
fail:
    // failed to find library
    lua_pushnil(L);
    return 1;
}
static int _debug( lua_State *L )
{
    const char* msg = luaL_checkstring(L, 1);
    lua_pop( L, 1 );
    U_PrintLn( (char*)msg);
    return 0;
}
static int _print_serial( lua_State *L )
{
    Caw_send_luachunk( (char*)luaL_checkstring(L, 1) );
    lua_pop( L, 1 );
    return 0;
}
static int _bootloader( lua_State *L )
{
    bootloader_enter();
    return 0;
}
static int _go_toward( lua_State *L )
{
    //const char* shape = luaL_checkstring(L, 4);
    S_toward( luaL_checkinteger(L, 1)-1 // C is zero-based
            , luaL_checknumber(L, 2)
            , luaL_checknumber(L, 3) * 1000.0
            , SHAPE_Linear // Shape_t
            , L_handle_toward
            );
    lua_pop( L, 4 );
    return 0;
}
static int _get_state( lua_State *L )
{
    float s = S_get_state( luaL_checkinteger(L, 1)-1 );
    lua_pop( L, 1 );
    lua_pushnumber( L
                  , s // testing if functional style causing issues?
                  );
    return 1;
}
static int _io_get_input( lua_State *L )
{
    float adc = IO_GetADC( luaL_checkinteger(L, 1)-1 );
    lua_pop( L, 1 );
    lua_pushnumber( L, adc );
    return 1;
}
static int _send_usb( lua_State *L )
{
    // pattern match on type: handle values vs strings vs chunk
    const char* msg = luaL_checkstring(L, 1);
    lua_pop( L, 1 );
    uint32_t len = strlen(msg);
    Caw_send_raw( (uint8_t*) msg, len );
    return 0;
}
static int _send_ii( lua_State *L )
{
    // pattern match on broadcast vs query
    uint8_t istate = 4;
    II_broadcast( II_FOLLOW, 1, &istate, 1 );
    return 0;
}
static int _set_ii_addr( lua_State *L )
{
    // pattern match on broadcast vs query
    uint8_t istate = 4;
    II_broadcast( II_FOLLOW, 1, &istate, 1 );
    return 0;
}
static int _metro_start( lua_State* L )
{
    static int idx = 0;
    float seconds = -1.0; // metro will re-use previous value
    int count = -1; // default: infinite
    int stage = 0;

    int nargs = lua_gettop(L);
    if (nargs > 0) { idx = (int) luaL_checkinteger(L, 1) - 1; } // 1-ix'd
    if (nargs > 1) { seconds = (float)luaL_checknumber(L, 2); }
    if (nargs > 2) { count = (int)luaL_checkinteger(L, 3); }
    if (nargs > 3) { stage = (int)luaL_checkinteger(L, 4) - 1; } // 1-ix'd
    lua_pop( L, 4 );

    metro_start( idx, seconds, count, stage ); // TODO implement in C
    lua_settop(L, 0);
    return 0;
}
static int _metro_stop( lua_State* L )
{
    if( lua_gettop(L) != 1 ){ return luaL_error(L, "wrong number of arguments"); }

    int idx = (int)luaL_checkinteger(L, 1) - 1; // 1-ix'd
    lua_pop( L, 1 );
    metro_stop(idx); // TODO implement in C
    lua_settop(L, 0);
    return 0;
}
static int _metro_set_time( lua_State* L )
{
    if( lua_gettop(L) != 2 ){ return luaL_error(L, "wrong number of arguments"); }

    int idx = (int)luaL_checkinteger(L, 1) - 1; // 1-ix'd
    float sec = (float) luaL_checknumber(L, 2);
    lua_pop( L, 2 );
    metro_set_time(idx, sec); // TODO implement in C
    lua_settop(L, 0);
    return 0;
}

// array of all the available functions
static const struct luaL_Reg libCrow[]=
        // bootstrap
    { { "c_dofile"       , _dofile           }
    , { "debug_usart"    , _debug            }
    , { "print_serial"   , _print_serial     }
        // system
    , { "sys_bootloader" , _bootloader       }
    //, { "sys_cpu_load"   , _sys_cpu          }
        // io
    , { "go_toward"      , _go_toward        }
    , { "get_state"      , _get_state        }
    , { "io_get_input"   , _io_get_input     }
        // usb
    , { "send_usb"       , _send_usb         }
        // i2c
    , { "send_ii"        , _send_ii          }
    , { "set_ii_addr"    , _set_ii_addr      }
        // metro
    , { "metro_start"    , _metro_start      }
    , { "metro_stop"     , _metro_stop       }
    , { "metro_set_time" , _metro_set_time   }

    , { NULL             , NULL              }
    };
// make functions available to lua
static void Lua_linkctolua( lua_State *L )
{
    // Make C fns available to Lua
    uint8_t fn = 0;
    while( libCrow[fn].func != NULL ){
        lua_pushcfunction( L, libCrow[fn].func );
        lua_setglobal( L, libCrow[fn].name );
        fn++;
    }
}

static uint8_t Lua_eval( lua_State* L, const char* script, ErrorHandler_t errfn ){
    int error;
    if( (error = luaL_loadstring( L, script ) || lua_pcall( L, 0, 0, 0 )) ){
        //(*errfn)( (char*)lua_tostring( L, -1 ) );
        Caw_send_luachunk( (char*)lua_tostring( L, -1 ) );
        lua_pop( L, 1 );
        switch( error ){
            case LUA_ERRSYNTAX: U_PrintLn("!load script: syntax"); break;
            case LUA_ERRMEM:    U_PrintLn("!load script: memory"); break;
            case LUA_ERRRUN:    U_PrintLn("!exec script: runtime"); break;
            case LUA_ERRERR:    U_PrintLn("!exec script: err in err handler"); break;
            default: break;
        }
        return 1;
    }
    return 0;
}

void Lua_crowbegin( void )
{
    // Call init() function
    // This is all we need to do -> the rest should flow back from Lua
    // The only callback->Lua *not* declared in Lua is a received command over USB
    U_PrintLn("init()"); // call in C to avoid user seeing in lua
    lua_getglobal(L,"init");
    lua_pcall(L,0,0,0);
}

// TODO the repl/state/reception logic should be its own file
//
// private declarations for repl
void Lua_repl_mode( L_repl_mode mode )
{
    repl_mode = mode;
    if( repl_mode == REPL_reception ){
        // begin a new transmission
        Lua_new_script_buffer();
    } else { // end of a transmission
        if( !Lua_eval( L, new_script, Caw_send_luaerror ) ){ // successful load
            // TODO if we're setting init() should check it doesn't crash
            *p_new_script = '\0'; // always end in null
            if( Flash_write_user_script( new_script
                                       , (uint32_t)(p_new_script - new_script)
                                       ) ){
                Caw_send_luachunk("flash write failed");
            }
        }
        free(new_script); // cleanup memory
    }
}

void Lua_repl( char* buf, uint32_t len, ErrorHandler_t errfn )
{
    if( repl_mode == REPL_normal ){
        Lua_eval( L, buf, errfn );
    } else {
        Lua_receive_script( buf, len, errfn );
    }
}

static void Lua_new_script_buffer( void )
{
    // TODO call to Lua to free resources from current script
    U_PrintLn("new transmission");
    new_script = malloc(0x3FFF); // allocate 16kB (or 0xFFFF, 64kb?)
    if(new_script == NULL){
        Caw_send_luachunk("!script: out of memory");
        //(*errfn)("!script: out of memory");
        return; // how to deal with this situation?
        // FIXME: should respond over usb stating out of memory?
        //        try allocating a smaller amount and hope it fits?
        //        retry?
    }
    p_new_script = new_script;
}

void Lua_receive_script( char* buf, uint32_t len, ErrorHandler_t errfn )
{
    memcpy( p_new_script, buf, len );
    p_new_script += len;
    p_new_script = '\0';
}

// Public Callbacks from C to Lua
void L_handle_toward( int id )
{
    lua_getglobal(L, "toward_handler");
    lua_pushinteger(L, id+1); // 1-ix'd
    if( lua_pcall(L, 1, 0, 0) != LUA_OK ){
        //U_PrintLn("error running toward_handler");
        Caw_send_luachunk("error running toward_handler");
        U_PrintLn( (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }
}

// probably need no static, and make this extern in header for access from metro lib
void L_handle_metro( const int id, const int stage)
{
    lua_getglobal(L, "metro_handler");
    lua_pushinteger(L, id+1);    // 1-ix'd
    lua_pushinteger(L, stage+1); // 1-ix'd
    if( lua_pcall(L, 2, 0, 0) != LUA_OK ){
        //U_Print("error running "); U_PrintLn("metro_handler");
        Caw_send_luachunk("error running metro_handler");
        U_PrintLn( (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }

}
