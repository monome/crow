// sketch of a generated C-header for crow-ii device (JF)

static uint8_t _addrs[] = {112, 117};
static uint8_t _addr = 112;
static uint8_t _ix = 1;

// configures the static variable for the provided module
// setting which index of address will be communicated
// with. used in cases where the same device can have
// multiple addresses for simultaneous access
static int ii_mod_setaddress( lua_State *L ){
    // stack: 1 = self
    //        2 = index of address (1-based)
    //        3 = extracted lua name
    lua_getfield(L, 1, "name");
    const char* mod_name = luaL_checkstring(L, 3);
    int address = luaL_checkinteger(L, 2);

    // find array length of addrs for the named module
    // update the variable for that module
    // or set to default if invalid

    lua_settop(L, 1); // leave self on stack
    return 1;
}

// core __index metamethod for the provided module.
// first looks up the module
    // this can be baked into the provided self
    // so can just be a pointer deref
// then looks up the field
    // this is the (slow) string search through
    // our generated tables of strings.
    // if speed is a concern (unlikely) this is a great
    // place to speed up ii cmd management
    // esp for txo which has 75(!) cmds
// saves the identifier for the message statically to
// be used when the next message is sent.
// then returns a generic Lua function that simply gathers
// arguments, and calls back to C to execute it.
// that follow-on func is some variation of _ii_lead
// but simplified bc it gets address & cmd from here.

static int query_to_cmd(_* module, const char* str){
    // lookup in *module from *str to it's cmd id.
}

static int ii_mod_query( lua_State *L ){
    // generic function
    int query_cmd = query_to_cmd(_module, luaL_checkstring(L, 1));
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
    if( ii_leader_enqueue( _ // address
                         , query_cmd // command
                         , data
                         ) ){ printf("ii_lead failed\n"); }
    lua_settop(L, 0);
    return 0;
}

static int ii_mod_command( lua_State *L ){
    // generic function
    float data[4] = {0,0,0,0}; // always zero out data
// TODO lookup number of args & arity check
// then only copy that number (warn of over/underflow)
    int nargs = lua_gettop(L);
    if(nargs > 4) nargs = 4; // limit to 4 ii arguments
    for(int i=0; i<nargs; i++ ){
        data[i] = luaL_checknumber(L, i+1); // 1-based
    }
    // address & command are local & static
    if( ii_leader_enqueue( _ // address
                         , _ // command
                         , data
                         ) ){ printf("ii_lead failed\n"); }
    lua_settop(L, 0);
    return 0;
}

static int ii_mod_index( lua_State *L ){
    // handle .help()
    // handle .event (return the generic fn as it doesn't exist)
        // useful for hand-testing your event without worrying
        // about the actual ii layer.
        // will need to be unique per mod as the string
        // for 'tell must be baked into the function
    // handle .get() -> will give a diff generic func
        // do this before main cmds as we have to do another lookup on the string
        // as it needs to collect the string-name
        // then look it up when we call back via _ii_lead*
    // handle rest of the list
        // will return the simple arg gatherer fn
        // must set:
            // number of args (arity check)
            // address
            // cmd (for setters)
        // ideally this is just a pointer to a struct
}