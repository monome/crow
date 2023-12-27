#include "l_crowlib.h"

#include "../ll/tp.h"

static int l_tp_power12(lua_State* L){
    TP_power12( luaL_checkinteger(L, 1) );
    lua_settop(L, 0);
    return 0;
}

static int l_tp_hub(lua_State* L){
    TP_hub( luaL_checkinteger(L, 1) );
    lua_settop(L, 0);
    return 0;
}

static int l_tp_dout(lua_State* L){
    TP_dout( luaL_checkinteger(L, 1)
           , luaL_checkinteger(L, 2) );
    lua_settop(L, 0);
    return 0;
}

static int l_tp_get_module_id(lua_State* L){
    lua_pushinteger( L, TP_get_module_id() );
    return 1;
}

static int l_tp_debug_led(lua_State* L){
    TP_debug_led( luaL_checkinteger(L, 1)
                , luaL_checkinteger(L, 2) );
    lua_settop(L, 0);
    return 0;
}

static int l_tp_cherry_state(lua_State* L){
    int index = luaL_checkinteger(L, 1);
    lua_pushinteger( L, TP_cherry_state(index) );
    return 1;
}

static int l_tp_dac_mux_1(lua_State* L){
    TP_dac_mux_1( luaL_checkinteger(L, 1) );
    lua_settop(L, 0);
    return 0;
}

static int l_tp_dac_mux_2(lua_State* L){
    TP_dac_mux_2( luaL_checkinteger(L, 1) );
    lua_settop(L, 0);
    return 0;
}

static int l_tp_jack_direction(lua_State* L){
    TP_jack_direction( luaL_checkinteger(L, 1) );
    lua_settop(L, 0);
    return 0;
}

static int l_tp_dac(lua_State* L){
    TP_dac( luaL_checkinteger(L, 1), luaL_checknumber(L, 2) );
    lua_settop(L, 0);
    return 0;
}

// array of all the available functions
static const struct luaL_Reg lib_test[]=
    { { "tp_power12"       , l_tp_power12       }
    , { "tp_hub"           , l_tp_hub           }
    , { "tp_dout"          , l_tp_dout          }
    , { "tp_get_module_id" , l_tp_get_module_id }
    , { "tp_debug_led"     , l_tp_debug_led     }
    , { "tp_cherry_state"  , l_tp_cherry_state  }
    , { "tp_dacmux1"       , l_tp_dac_mux_1     }
    , { "tp_dacmux2"       , l_tp_dac_mux_2     }
    , { "tp_jack_dir"      , l_tp_jack_direction}
    , { "tp_dac"           , l_tp_dac           }
    , { NULL               , NULL               }
    };

static void linkctolua( lua_State *L )
{
    // Make C fns available to Lua
    uint8_t fn = 0;
    while( lib_test[fn].func != NULL ){
        lua_register( L, lib_test[fn].name, lib_test[fn].func );
        fn++;
    }
}

void l_test_preload(lua_State* L){
    // load C funcs into lua env
    linkctolua(L);
}
