#include "lib/lualink.h"

#include <string.h> // strcmp(), strlen()

// Lua itself
//#include "../submodules/lua/src/lua.h" // in header
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

// Hardware IO
#include "lib/slopes.h"     // S_toward
#include "lib/detect.h"     // Detect*
#include "lib/caw.h"        // Caw_send_*()
#include "lib/ii.h"         // II_*()
#include "lib/bootloader.h" // bootloader_enter()
#include "lib/metro.h"      // metro_start() metro_stop() metro_set_time()
#include "lib/io.h"         // IO_GetADC()
#include "../ll/random.h"      // Random_Get()

// Lua libs wrapped in C-headers: Note the extra '.h'
#include "lua/bootstrap.lua.h" // MUST LOAD THIS MANUALLY FIRST
#include "lua/crowlib.lua.h"
#include "lua/asl.lua.h"
#include "lua/asllib.lua.h"
#include "lua/metro.lua.h"
#include "lua/input.lua.h"
#include "lua/output.lua.h"
#include "lua/ii.lua.h"
#include "build/iihelp.lua.h"    // generated lua stub for loading i2c modules

#include "build/ii_lualink.h" // generated C header for linking to lua

const struct lua_lib_locator Lua_libs[] =
    { { "lua_crowlib", lua_crowlib }
    , { "lua_asl"    , lua_asl     }
    , { "lua_asllib" , lua_asllib  }
    , { "lua_metro"  , lua_metro   }
    , { "lua_input"  , lua_input   }
    , { "lua_output" , lua_output  }
    , { "lua_ii"     , lua_ii      }
    , { "build_iihelp", build_iihelp }
    , { NULL         , NULL        }
    };

// Basic crow script
#include "lua/default.lua.h"

// Private prototypes
static void Lua_linkctolua( lua_State* L );
static float Lua_check_memory( void );

void _printf(char* error_message)
{
    printf("%s\n",error_message);
}

lua_State* L; // global access for 'reset-environment'

// Public functions
lua_State* Lua_Init(void)
{
    L = luaL_newstate();
    luaL_openlibs(L);
    Lua_linkctolua(L);
    Lua_eval(L, lua_bootstrap
              , strlen(lua_bootstrap)
              , _printf
              ); // redefine dofile(), print(), load crowlib
    return L;
}

void Lua_load_default_script( void )
{
    Lua_eval(L, lua_default
              , strlen(lua_default)
              , _printf
              );
}

void Lua_DeInit(void)
{
    lua_close(L);
}

// C-fns accessible to lua

// NB these static functions are prefixed  with '_'
// to avoid shadowing similar-named extern functions in other modules
// and also to distinguish from extern 'L_' functions.

static int _find_lib( const struct lua_lib_locator* lib, const char* name )
{
    uint8_t i = 0;
    while( lib[i].addr_of_luacode != NULL ){
        if( !strcmp( name, lib[i].name ) ){ // if the strings match
            if( luaL_dostring( L, lib[i].addr_of_luacode ) ){
                printf("can't load library: %s\n", (char*)lib[i].name );
                // lua error
                printf( "%s\n", (char*)lua_tostring( L, -1 ) );
                lua_pop( L, 1 );
                return -1; // error
            }
            return 1; // table is left on the stack as retval
        }
        i++;
    }
    return 0; // not found
}

static int _dofile( lua_State *L )
{
    const char* l_name = luaL_checkstring(L, 1);
    lua_pop( L, 1 );
    switch( _find_lib( Lua_libs, l_name ) ){
        case -1: goto fail;
        case 1: return 1;
        default: break;
    }
    switch( _find_lib( Lua_ii_libs, l_name ) ){
        case -1: goto fail;
        case 1: return 1;
        default: break;
    }
    printf("can't find library: %s\n", (char*)l_name);
fail:
    lua_pushnil(L);
    return 1;
}
static int _debug( lua_State *L )
{
    const char* msg = luaL_checkstring(L, 1);
    lua_pop( L, 1 );
    printf( "%s\n",(char*)msg);
    lua_settop(L, 0);
    return 0;
}
static int _print_serial( lua_State *L )
{
    Caw_send_luachunk( (char*)luaL_checkstring(L, 1) );
    lua_pop( L, 1 );
    lua_settop(L, 0);
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
    lua_settop(L, 0);
    return 0;
}
static int _get_state( lua_State *L )
{
    float s = S_get_state( luaL_checkinteger(L, 1)-1 );
    lua_pop( L, 1 );
    lua_pushnumber( L, s );
    return 1;
}
static int _io_get_input( lua_State *L )
{
    float adc = IO_GetADC( luaL_checkinteger(L, 1)-1 );
    lua_pop( L, 1 );
    lua_pushnumber( L, adc );
    return 1;
}
static int _set_input_none( lua_State *L )
{
    uint8_t ix = luaL_checkinteger(L, 1)-1;
    Detect_t* d = Detect_ix_to_p( ix ); // Lua is 1-based
    if( d != NULL ){ // valid index
        Detect_none( d );
        Metro_stop( ix );
    }
    lua_pop( L, 1 );
    lua_settop(L, 0);
    return 0;
}
static int _set_input_stream( lua_State *L )
{
    uint8_t ix = luaL_checkinteger(L, 1)-1;
    Detect_t* d = Detect_ix_to_p( ix ); // Lua is 1-based
    if( d != NULL ){ // valid index
        Detect_none( d );
        Metro_start( ix
                   , luaL_checknumber(L, 2)
                   , -1
                   , 0
                   );
    }
    lua_pop( L, 2 );
    lua_settop(L, 0);
    return 0;
}
static int _set_input_change( lua_State *L )
{
    uint8_t ix = luaL_checkinteger(L, 1)-1;
    Detect_t* d = Detect_ix_to_p( ix ); // Lua is 1-based
    if( d != NULL ){ // valid index
        Metro_stop( ix );
        Detect_change( d
                     , L_handle_change
                     , luaL_checknumber(L, 2)
                     , luaL_checknumber(L, 3)
                     , Detect_str_to_dir( luaL_checkstring(L, 4) )
                     );
    }
    lua_pop( L, 4 );
    lua_settop(L, 0);
    return 0;
}
static int _send_usb( lua_State *L )
{
    // pattern match on type: handle values vs strings vs chunk
    const char* msg = luaL_checkstring(L, 1);
    lua_pop( L, 1 );
    uint32_t len = strlen(msg);
    Caw_send_raw( (uint8_t*) msg, len );
    lua_settop(L, 0);
    return 0;
}

static int _ii_list_modules( lua_State *L )
{
    Caw_send_luachunk( (char*)II_list_modules() );
    printf( "%s\n",(char*)II_list_modules() );
    return 0;
}

static int _ii_list_commands( lua_State *L )
{
    uint8_t address = luaL_checkinteger(L, 1);
    printf("i2c help %i\n", address);
    //Caw_send_luachunk( (char*)II_list_modules() );
    return 0;
}
static int _ii_set( lua_State *L )
{
    printf("lua ii broadcast\n");

    // FIXME: 4 is max number of arguments. is this ok?
    float data[4] = {0,0,0,0}; // always zero out data
    int nargs = lua_gettop(L);
    if( nargs > 2
     && nargs <= 6 ){
        for( int i=0; i<(nargs-2); i++ ){
            data[i] = luaL_checknumber(L, i+3); // 1-ix'd
        }
    }
    II_broadcast( luaL_checkinteger(L, 1) // address
                , luaL_checkinteger(L, 2) // command
                , data
                );
    lua_settop(L, 0);
    return 0;
}
static int _ii_get( lua_State *L )
{
    printf("lua ii query\n");
    float data[4] = {0,0,0,0}; // always zero out data
    int nargs = lua_gettop(L);
    if( nargs > 2
     && nargs <= 6 ){
        for( int i=0; i<(nargs-2); i++ ){
            data[i] = luaL_checknumber(L, i+3); // 1-ix'd
        }
    }
    II_query( luaL_checkinteger(L, 1) // address
            , luaL_checkinteger(L, 2) // command
            , data
            );
    lua_settop(L, 0);
    return 0;
}
static int _ii_address( lua_State *L )
{
    II_set_address( luaL_checkinteger(L, 1) );
    lua_pop( L, 1 );
    lua_settop(L, 0);
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

    Metro_start( idx+2, seconds, count, stage ); // +2 for adc
    lua_settop(L, 0);
    return 0;
}
static int _metro_stop( lua_State* L )
{
    if( lua_gettop(L) != 1 ){ return luaL_error(L, "wrong number of arguments"); }

    int idx = (int)luaL_checkinteger(L, 1) - 1; // 1-ix'd
    lua_pop( L, 1 );
    Metro_stop(idx+2); // +2 for adc
    lua_settop(L, 0);
    return 0;
}
static int _metro_set_time( lua_State* L )
{
    if( lua_gettop(L) != 2 ){ return luaL_error(L, "wrong number of arguments"); }

    int idx = (int)luaL_checkinteger(L, 1) - 1; // 1-ix'd
    float sec = (float) luaL_checknumber(L, 2);
    lua_pop( L, 2 );
    Metro_set_time(idx+2, sec); // +2 for adc
    lua_settop(L, 0);
    return 0;
}

static int _random_get( lua_State* L )
{
    lua_pushnumber( L, Random_Get() );
    return 1;
}

// array of all the available functions
static const struct luaL_Reg libCrow[]=
        // bootstrap
    { { "c_dofile"         , _dofile           }
    , { "debug_usart"      , _debug            }
    , { "print_serial"     , _print_serial     }
        // system
    , { "sys_bootloader"   , _bootloader       }
    //, { "sys_cpu_load"     , _sys_cpu          }
        // io
    , { "go_toward"        , _go_toward        }
    , { "get_state"        , _get_state        }
    , { "io_get_input"     , _io_get_input     }
    , { "set_input_none"   , _set_input_none   }
    , { "set_input_stream" , _set_input_stream }
    , { "set_input_change" , _set_input_change }
        // usb
    , { "send_usb"         , _send_usb         }
        // i2c
    , { "ii_list_modules"  , _ii_list_modules  }
    , { "ii_list_commands" , _ii_list_commands }
    , { "ii_set"           , _ii_set           }
    , { "ii_get"           , _ii_get           }
    , { "ii_address"       , _ii_address       }
        // metro
    , { "metro_start"      , _metro_start      }
    , { "metro_stop"       , _metro_stop       }
    , { "metro_set_time"   , _metro_set_time   }
        // random
    , { "random_get"       , _random_get       }

    , { NULL               , NULL              }
    };
// make functions available to lua
static void Lua_linkctolua( lua_State *L )
{
    // Make C fns available to Lua
    uint8_t fn = 0;
    while( libCrow[fn].func != NULL ){
        lua_register( L, libCrow[fn].name, libCrow[fn].func );
        fn++;
    }
}

uint8_t Lua_eval( lua_State*     L
                , const char*    script
                , size_t         script_len
                , ErrorHandler_t errfn
                ){
    int error;
    if( (error = luaL_loadbuffer( L, script, script_len, "eval" )
              || lua_pcall( L, 0, 0, 0 )
        ) ){
        //(*errfn)( (char*)lua_tostring( L, -1 ) );
        Caw_send_luachunk( (char*)lua_tostring( L, -1 ) );
        lua_pop( L, 1 );
        switch( error ){
            case LUA_ERRSYNTAX: printf("!load script: syntax\n"); break;
            case LUA_ERRMEM:    printf("!load script: memory\n"); break;
            case LUA_ERRRUN:    printf("!exec script: runtime\n"); break;
            case LUA_ERRERR:    printf("!exec script: err in err handler\n"); break;
            default: break;
        }
        return 1;
    }
    return 0;
}

static float Lua_check_memory( void )
{
    lua_getglobal(L,"collectgarbage");
    lua_pushstring(L, "count"); // 1-ix'd
    lua_pcall(L,1,1,0);
    float mem = luaL_checknumber(L, 1);
    lua_pop(L,1);
    return mem;
}

void Lua_crowbegin( void )
{
    printf("init()\n"); // call in C to avoid user seeing in lua
    lua_getglobal(L,"init");
    lua_pcall(L,0,0,0);
}

// Public Callbacks from C to Lua
void L_handle_toward( int id )
{
    lua_getglobal(L, "toward_handler");
    lua_pushinteger(L, id+1); // 1-ix'd
    if( lua_pcall(L, 1, 0, 0) != LUA_OK ){
        Caw_send_luachunk("error running toward_handler");
        printf( "%s\n", (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }
}

void L_handle_metro( const int id, const int stage)
{
    lua_getglobal(L, "metro_handler");
    lua_pushinteger(L, id+1 -2); // 1-ix'd, less 2 for adc rebase
    lua_pushinteger(L, stage+1); // 1-ix'd
    if( lua_pcall(L, 2, 0, 0) != LUA_OK ){
        Caw_send_luachunk("error running metro_handler");
        Caw_send_luachunk( (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }
}

void L_handle_in_stream( int id, float value )
{
    lua_getglobal(L, "stream_handler");
    lua_pushinteger(L, id+1); // 1-ix'd
    if( value > 10.0 ){ value = 10.0; }
    if( value < -5.0 ){ value = -5.0; }
    lua_pushnumber(L, value);
    if( lua_pcall(L, 2, 0, 0) != LUA_OK ){
        Caw_send_luachunk("error: input stream");
        Caw_send_luachunk( (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }
}

void L_handle_change( int id, float state )
{
    lua_getglobal(L, "change_handler");
    lua_pushinteger(L, id+1); // 1-ix'd
    lua_pushnumber(L, state);
    if( lua_pcall(L, 2, 0, 0) != LUA_OK ){
        printf("ch er\n");
        Caw_send_luachunk("error: input change");
        Caw_send_luachunk( (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }
}

void L_handle_ii( uint8_t address, uint8_t cmd, float data )
{
    lua_getglobal(L, "ii_handler");
    lua_pushinteger(L, address);
    lua_pushinteger(L, cmd);
    lua_pushnumber(L, data); // TODO currently limited to single retval
    if( lua_pcall(L, 3, 0, 0) != LUA_OK ){
        printf("ii ev err\n");
        Caw_send_luachunk("error: ii event");
        Caw_send_luachunk( (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }
}

void L_handle_iiself( uint8_t cmd, int args, float* data )
{
    lua_getglobal(L, "ii_self_handler");
    lua_pushinteger(L, cmd);
    int a = args;
    while(a-- > 0){
        lua_pushnumber(L, *data++);
    }
    if( lua_pcall(L, 1+args, 0, 0) != LUA_OK ){
        printf("ii ev err\n");
        Caw_send_luachunk("error: ii event");
        Caw_send_luachunk( (char*)lua_tostring(L, -1) );
        lua_pop( L, 1 );
    }
}
