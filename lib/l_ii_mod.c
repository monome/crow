

/// MUST SEARCH FOR ALL FIXMEs BEFORE FINALIZING!!!!!!!



#include "l_ii_mod.h"
#include <stddef.h>
#include <string.h>

#include "caw.h"
#include "ii.h"

////////////////////////////////////////////////
// code-generated data
// this should be in a separate file

static const ii_mod_cmds_t ii_mod_jf_cmds[] =
    {{.cmd = 1, .name = "trigger"}
    ,{.cmd = 4, .name = "transpose"}
    ,{.cmd = 14, .name = "address"}
    };
// data for each module
static ii_box_t ii_box_jf =
    { .addr          = 112 // this is dynamic!
    , .name          = "jf"
    , .addresses     = {112,117}
    , .commands      = ii_mod_jf_cmds
    , .command_count = 3
    };
// list of all module structs
static ii_box_t* ii_mods[] = {&ii_box_jf};

////////////////////////////////////////////////
// global vars
// set as commands are executed
static uint8_t active_address = 0x0;
static uint8_t active_cmd = 0x0;
static ii_box_t* active_box = NULL;


////////////////////////////////////////////////
// search functions
// peer into the generated structures


////////////////////////////////////////////////
// TODO /////////////////////////////////////////////

static ii_box_t* find_mod_struct_by_address(uint8_t addr){
    return &ii_box_jf; // FIXME
}

static ii_box_t* find_mod_struct_by_name(const char* name, size_t len){
    // TODO string search of tables
    return &ii_box_jf; // FIXME
}

static const char* find_cmd_name(ii_box_t* box, uint8_t cmd){
    // FIXME return the string-name of the cmd in the provided wrapper
    // will only be for getters so start search at end and work backward
    return "ramp";
}

static int query_to_cmd(ii_box_t* box, const char* str){
    // lookup in *wrapper from *str to it's cmd id.
    return 145; // FIXME hard-coded .get('ramp')
}


////////////////////////////////////
// completed!

static int string_to_cmd(ii_box_t* box, const char* str){
    const ii_mod_cmds_t* cmds = box->commands;
    int i=0;
    uint8_t cmd = 0xFF;
    while(i < box->command_count){
        if(strcmp(cmds[i].name, str) == 0){
            cmd = cmds[i].cmd;
            break;
        }
        i++;
    }
    return cmd; // FIXME hard-coded .trigger
}

static ii_box_t* find_mod_struct_from_table(lua_State* L, int self_stack_index){
    lua_getfield(L, self_stack_index, "_name"); // push the device name onto TOS
    size_t len = 0;
    const char* name = lua_tolstring(L, -1, &len);
    // len now contains the length of the string. use for fast lookup
    return find_mod_struct_by_name(name, len);
}

static int find_addr_ix(ii_box_t* box, int addr_now){
    // returns the 1-based index into .addresses that is currently in .addr
    uint8_t* addrs = box->addresses; 
    for(int i=0; i<II_MAX_ADDRESSES; i++){
        if(addr_now == addrs[i]){
            return i+1; // lua is 1-based
        }
    }
    return -1; // on failure (shouldn't happen)
}


////////////////////////////////////////////////

static int l_ii_setaddress( lua_State* L ){
    uint8_t addr_ix = (uint8_t)luaL_checkinteger(L, 2) - 1; // lua is 1-based
    if(addr_ix >= II_MAX_ADDRESSES) addr_ix = 0; // protect against out of bounds lookup

    ii_box_t* box = find_mod_struct_from_table(L, 1);

    // now set the current address in thebox 
    int new_addr = box->addresses[addr_ix];
    if(new_addr){ // protect against 0 (ie. invalid addresses)
        box->addr = new_addr;
    }

    lua_settop(L, 1); // returns self which still sits at index 1
    return 1;
}

/////////////////////////////////////////////////////
// generic fns for executing __index metamethod of the provided module

static int __index_help( lua_State* L ){
    // NOTE: active_address must already be set
    printf("i2c help %i\n", active_address);
    Caw_stream_constchar( ii_list_cmds(active_address) );
    lua_settop(L, 0);
    return 0;
}

static int __index_event( lua_State* L ){
    // this is the generic event printer

    double data = luaL_checknumber(L, 2);
    lua_getfield(L, 1, "name");
    const char* name = luaL_checkstring(L, -1);
    lua_getfield(L, 1, "device");
    int device = luaL_checkinteger(L, -1);
    lua_getfield(L, 1, "arg");
    double arg = luaL_checknumber(L, -1);

    Caw_printf( "^^ii.%s({name=[[%s]],device=%i,arg=%g},%g)"
        , active_box->name
        , name
        , device
        , arg
        , data);

    lua_settop(L, 0);
    return 0;
}

static int __index_get( lua_State* L ){
    // lua: ii_lead with check on validity of cmd
    int query_cmd = query_to_cmd(active_box, luaL_checkstring(L, 1));
    // this should select a cmd struct so we can arity check
    float data[4] = {0,0,0,0}; // always zero out data
// TODO lookup number of args & arity check
// then only copy that number (warn of over/underflow)
    int nargs = lua_gettop(L) - 1; // reduce for 'string'
    if(nargs > 4) nargs = 4; // limit to 4 ii arguments
    for(int i=0; i<nargs; i++ ){
        data[i] = luaL_checknumber(L, i+2); // 1-based (plus space for 'string)
    }
    // address & command are local & static
    if( ii_leader_enqueue( active_address // address
                         , query_cmd // command
                         , data
                         ) ){ printf("ii_lead failed\n"); }
    lua_settop(L, 0);
    return 0;
}

static int __index_command( lua_State* L ){
    // lua: ii_lead
    float data[4] = {0,0,0,0};
    int nargs = lua_gettop(L);
// TODO arity check nargs
// ->error on too few
// ->warn on too many
    if(nargs > 4) nargs = 4; // limit to 4 ii arguments
    for(int i=0; i<nargs; i++ ){
        data[i] = luaL_checknumber(L, i+1); // 1-based
    }
    // address & command are local & static
    if( ii_leader_enqueue( active_address
                         , active_cmd
                         , data
                         ) ){ printf("ii_lead failed\n"); }
    lua_settop(L, 0);
    return 0;
}

static int l_ii_index( lua_State* L ){
    // returns a C function depending on the action
    // @1: self (eg. ii.jf{} table)
    // @2: command-string (eg. event, help, get or a cmd-name)

    // find which module we're talking about
    active_box = find_mod_struct_from_table(L, 1);
    active_address = active_box->addr; // default to the actual set address

    // grab the command-string name
    size_t len = 0;
    const char* name = lua_tolstring(L, 2, &len);

    lua_settop(L, 0); // clear stack

    // search for the known strings
    if(strcmp(name, "help") == 0){
        // we use the first address for help lookup
        active_address = active_box->addresses[0];
        lua_pushcfunction(L, __index_help);
    } else if(strcmp(name, "event") == 0){
        lua_pushcfunction(L, __index_event);
    } else if(strcmp(name, "get") == 0){
        lua_pushcfunction(L, __index_get);
    } else { // and now search for a command
        active_cmd = string_to_cmd(active_box, name);
        if(active_cmd == 0xFF){
            return luaL_error(L, "can't find: ii.%s.%s", active_box->name, name);
        }
        lua_pushcfunction(L, __index_command);
    }
    return 1;
}

static int l_ii_cmd_from_ix( lua_State* L ){
    uint8_t addr = luaL_checkinteger(L, 1);
    uint8_t cmd = luaL_checkinteger(L, 2);
    lua_settop(L,0);
    ii_box_t* box = find_mod_struct_by_address(addr);
    lua_pushstring(L, box->name);
    lua_pushstring(L, find_cmd_name(box, cmd)); // replace addr with struct*
    lua_pushinteger(L, find_addr_ix(box, addr)); // FIXME get real address
    return 3;
}

// array of all the available functions
static const struct luaL_Reg lib_ii_mod[]=
    { { "c_ii_setaddress" , l_ii_setaddress  }
    , { "c_ii_index"      , l_ii_index       }
    , { "c_ii_cmd"        , l_ii_cmd_from_ix }
    , { NULL              , NULL             }
    };

static void linkctolua( lua_State *L )
{
    // Make C fns available to Lua
    uint8_t fn = 0;
    while( lib_ii_mod[fn].func != NULL ){
        lua_register( L, lib_ii_mod[fn].name, lib_ii_mod[fn].func );
        fn++;
    }
}

void l_ii_mod_preload(lua_State* L){
    // load C funcs into lua env
    linkctolua(L);
}

