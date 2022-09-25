#include "l_bootstrap.h"

#include <string.h>

#include "l_crowlib.h"

#include "lua/crowlib.lua.h"
#include "lua/asl.lua.h"
#include "lua/asllib.lua.h"
#include "lua/clock.lua.h"
#include "lua/metro.lua.h"
#include "lua/public.lua.h"
#include "lua/input.lua.h"
#include "lua/output.lua.h"
#include "lua/ii.lua.h"
#include "build/iihelp.lua.h"    // generated lua stub for loading i2c modules
#include "lua/calibrate.lua.h"
#include "lua/sequins.lua.h"
#include "lua/quote.lua.h"

#include "build/ii_lualink.h" // generated C header for linking to lua

static int _writer(lua_State *L, const void *p, size_t sz, void *ud);
static int _load_chunk(lua_State* L, const char* code, int strip);
static int _open_lib( lua_State *L, const struct lua_lib_locator* lib, const char* name );

// mark the 3rd arg 'false' if you need to debug that library
const struct lua_lib_locator Lua_libs[] =
    { { "lua_crowlib"   , lua_crowlib   , true}
    , { "lua_asl"       , lua_asl       , true}
    , { "lua_asllib"    , lua_asllib    , true}
    , { "lua_clock"     , lua_clock     , true}
    , { "lua_metro"     , lua_metro     , true}
    , { "lua_input"     , lua_input     , true}
    , { "lua_output"    , lua_output    , true}
    , { "lua_public"    , lua_public    , true}
    , { "lua_ii"        , lua_ii        , true}
    , { "build_iihelp"  , build_iihelp  , true}
    , { "lua_calibrate" , lua_calibrate , true}
    , { "lua_sequins"   , lua_sequins   , true}
    , { "lua_quote"     , lua_quote     , true}
    , { NULL            , NULL          , true}
    };


void l_bootstrap_init(lua_State* L){
	// collectgarbage('setpause', 55)
	lua_gc(L, LUA_GCSETPAUSE, 55);
	lua_gc(L, LUA_GCSETSTEPMUL, 260);

	// dofile just calls c_dofile
	lua_getglobal(L, "c_dofile");
	lua_setglobal(L, "dofile");

	// TODO collect & implement much of crowlib here directly
	// _c = dofile('lua/crowlib.lua')
	lua_pushliteral(L, "lua/crowlib.lua");
	l_bootstrap_dofile(L); // hotrod without l_call
	lua_setglobal(L, "_c");

	// crow = _c
	lua_getglobal(L, "_c");
	lua_setglobal(L, "crow");

    // crowlib C extensions
    l_crowlib_init(L);

	// perform two full garbage collection cycles for full cleanup
	lua_gc(L, LUA_GCCOLLECT, 1);
	lua_gc(L, LUA_GCCOLLECT, 1);
}


int l_bootstrap_dofile(lua_State* L)
{
    const char* l_name = luaL_checkstring(L, 1);
    int l_len = strlen(l_name);
    if(l_len > 32) printf("FIXME bootstrap: filepath >32bytes!\n\r");

    // simple C version of "luapath_to_cpath"
    // l_name is a lua native path: "lua/asl.lua"
    char cname[32]; // 32bytes is more than enough for any path
    int p=0; // pointer into cname
    for(int i=0; i<l_len; i++){
    	switch(l_name[i]){
    		case '/':{ cname[p++] = '_'; } break;
    		case '.':{ cname[p++] = 0; } goto strcomplete;
    		default:{ cname[p++] = l_name[i]; } break;
    	}
    }
strcomplete:
    lua_pop( L, 1 );
    switch( _open_lib( L, Lua_libs, cname ) ){
        case -1: goto fail;
        case 1: return 1;
        default: break;
    }
    switch( _open_lib( L, Lua_ii_libs, cname ) ){
        case -1: goto fail;
        case 1: return 1;
        default: break;
    }
    printf("can't open library: %s\n", (char*)cname);
fail:
    lua_pushnil(L);
    return 1;
}






/////////// private defns

// this is a somewhat arbitrary size. must be big enough for the biggest library. 8kB was too small
#define SIZED_STRING_LEN 0x4000 // (16kB)
struct sized_string{
    char data[SIZED_STRING_LEN];
    int  len;
};

#define UNUSED(x) (void)(sizeof(x))
static int _writer(lua_State *L, const void *p, size_t sz, void *ud)
{
    UNUSED(L);
    struct sized_string* chunkstr = ud; /// need explicit cast?
    if(chunkstr->len + sz >= SIZED_STRING_LEN){
        printf("chunkstr too small.\n");
        return 1;
    }
    memcpy(&chunkstr->data[chunkstr->len], p, sz);
    chunkstr->len += sz;
    return 0;
}
#undef UNUSED

static int _load_chunk(lua_State* L, const char* code, int strip)
{
    int retval = 0;
    struct sized_string chunkstr = {.len = 0};
    { // scope lua_State to destroy it asap
        lua_State* LL=luaL_newstate();
        if( !LL ){ printf("luaL_newstate failed\n"); return 1; }
        if( luaL_loadstring(LL, code) ){
            printf("loadstring error\n");
            retval = 1;
            goto close_LL;
        }
        if( lua_dump(LL, _writer, &chunkstr, strip) ){
            printf("dump error\n");
            retval = 1;
            goto close_LL;
        }
close_LL:
        lua_close(LL);
    }
    luaL_loadbuffer(L, chunkstr.data, chunkstr.len, chunkstr.data); // load our compiled chunk
    return retval;
}

static int _open_lib( lua_State *L, const struct lua_lib_locator* lib, const char* name )
{
    uint8_t i = 0;
    while( lib[i].addr_of_luacode != NULL ){
        if( !strcmp( name, lib[i].name ) ){ // if the strings match
            if( _load_chunk(L, lib[i].addr_of_luacode, lib[i].stripped) ){
                printf("can't load library: %s\n", (char*)lib[i].name );
                printf( "%s\n", (char*)lua_tostring( L, -1 ) );
                lua_pop( L, 1 );
                return -1; // error
            }
            if( lua_pcall(L, 0, LUA_MULTRET, 0) ){
                printf("can't exec library: %s\n", (char*)lib[i].name );
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
