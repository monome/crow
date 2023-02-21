#include "l_metro.h"

#include "lib/lualink.h"    // Lua_call_usercode()
#include "lib/events.h"     // event_t event_post()
#include "lib/metro.h"      // Metro_stop_all()

typedef struct{
    float time;
    int   count;
    int   stage;
    int   id;
    bool  available;
    // lua_function action;
} Metro_obj;

#define MAX_METROS 8 // this is runtime initialized in Metro_Init()

static int available_metros = MAX_METROS;
static Metro_obj metros[MAX_METROS];

extern lua_State* L; // global access for 'reset-environment'

// private declarations
static bool can_alloc(void);
static int allocate(void);
static void L_handle_metro( event_t* e );
static void free_metro(int ix);
static void set_defaults(int ix);


// public interface via lua

// initialize the module itself
int l_metrolib_preinit( lua_State *L ){
    // initialize C-structs
    for(int i=0; i<MAX_METROS; i++){
        free_metro(i);
    }

    // think of this as the executable pass through the lua file
    // all the fn-defs are done automatically, but any allocation is here

    // allocate a lua table for each metro
    // table just has an event & lightuserdatum
    // then applies the metatable

    // place each metro table directly in the Metro table at it's index
    // this allows to skip a metamethod indirection & simplifies __index


    // CURRENTLY WE DO IT WITH THE metro.lua FILE
    // THIS CAN BE CEIFIED TO REDUCE RAM USAGE SOMEWHAT

    return 0;
}

// free all metros & return to defaults
int l_metro_free_all( lua_State *L ){
    Metro_stop_all();
    for(int i=0; i<MAX_METROS; i++){
        set_defaults(i);
    }
    available_metros = MAX_METROS;
    return 0;
}

// no args: allocate a metro and return the metro object
// 3 args: allocate & assign args to object
// table: allocate & assign by key name
int l_metro_init( lua_State *L )
{
    if(!can_alloc()){
        goto error;
    }

    int alloc = allocate();
    set_defaults(alloc); // baseline (incl. no args)
    Metro_obj* obj = &metros[alloc];

    int nargs = lua_gettop(L);
    if(nargs == 1 && lua_type(L, -1) == LUA_TTABLE){
        // table: allocate & assign by field name
        // put object table on TOS
        lua_getglobal(L, "metro"); // get metro table
        lua_pushinteger(L, obj->id+1); // lua is 1-based
        lua_gettable(L, -2); // leaves metro[n] table on TOS

        // check for event, time & count & set them in the table
        if(lua_getfield(L, 1, "time") == LUA_TNUMBER){
            float seconds = luaL_checknumber(L,-1);
            if(seconds >= 0.0){
                obj->time = (seconds < 0.0005) ? 0.0005 : seconds;
                Metro_set_time(obj->id, obj->time);
            }
        }
        lua_settop(L, 3); // reset for next table lookup
        if(lua_getfield(L, 1, "count") == LUA_TNUMBER){
            obj->count = luaL_checkinteger(L,-1);
            Metro_set_count(obj->id, obj->count);
        }
        lua_settop(L, 3); // reset for next table lookup
        if(lua_getfield(L, 1, "event") == LUA_TFUNCTION){
            //// lua stack:
            // 1: parameter table from lua script
            // 2: metro global object (ie metro itself)
            // 3: metro object (self) (ie metro[n])
            // 4: event function
            lua_pushstring(L,"event");
            // 5: 'event' string
            lua_rotate(L,4,1); // swap top 2 elements
            // 4: 'event' string
            // 5: event function
            lua_rawset(L,3);
        }
        lua_copy(L,3,1); // copy self metro object to 1st stack placement
    } else {
        // up to 3 args: allocate & assign args to object
        // first get the metro[n] self object
        lua_getglobal(L, "metro"); // get metro table
        lua_pushinteger(L, obj->id+1); // lua is 1-based
        lua_gettable(L, -2); // leaves metro[n] table on TOS
        int self_location = lua_gettop(L);
        if(nargs>=1){ // event
            lua_pushstring(L, "event");
            lua_pushinteger(L, 0); // placeholder for lua_copy
            lua_copy(L,1,-1); // overwrites TOS
            lua_rawset(L,-3); // writes value into table
        }
        if(nargs>=2){ // time
            float seconds = luaL_checknumber(L,2);
            if(seconds >= 0.0){
                obj->time = (seconds < 0.0005) ? 0.0005 : seconds;
                Metro_set_time(obj->id, obj->time);
            }
        }
        if(nargs>=3){ // count
            obj->count = luaL_checkinteger(L,3);
            Metro_set_count(obj->id, obj->count);
        }
        lua_copy(L,self_location,1); // copy self object into 1st place
    }
    lua_settop(L, 1); // abandon all but the metro object
    return 1;

error:
    luaL_error(L, "Metro.init: nothing available");
    lua_settop(L, 0);
    lua_pushnil(L);
    return 1;
}


// class-local actions
int l_metro_start( lua_State *L ){
    // args: self, (opt->) time, count stage
    if(!lua_istable(L,1)){
        luaL_error(L, "Metro.start: no self provided");
        goto error;
    }
    lua_getfield(L,1,"id"); // push id of metro onto stack (1-based)
    Metro_obj* obj = &metros[luaL_checkinteger(L,-1)-1]; // grab pointer to C struct. 0-based.
    lua_pop(L,1); // remove pointer from stack

    // apply optional args here
    int nargs = lua_gettop(L);
    if(nargs>=2){ // time
        float seconds = luaL_checknumber(L,2);
        if(seconds >= 0.0){
            obj->time = (seconds < 0.0005) ? 0.0005 : seconds;
            Metro_set_time(obj->id, obj->time);
        }
    }
    if(nargs>=3){ // count
        obj->count = luaL_checkinteger(L,3);
        Metro_set_count(obj->id, obj->count);
    }
    if(nargs>=4){ // stage
        obj->stage = luaL_checkinteger(L,4);
        Metro_set_stage(obj->id, obj->stage);
    }

    Metro_start(obj->id);
error:
    lua_settop(L,0);
    return 0;
}

int l_metro_stop( lua_State *L ){
    // args: self, ...
    if(!lua_istable(L,1)){
        luaL_error(L, "Metro.stop: no self provided");
        goto error;
    }
    lua_getfield(L,1,"id"); // push id of metro onto stack. 1-based
    Metro_obj* obj = &metros[luaL_checkinteger(L,-1)-1]; // grab pointer to C struct. 0-based
    lua_pop(L,1); // remove pointer from stack

    Metro_stop(obj->id);
error:
    lua_settop(L, 0);
    return 0;
}

// metamethods
int l_metro__index( lua_State *L ){
    // access methods of the metro object
    // query state of the metro object
    // args: self, fieldname

    // pull userdata out of self
    if(!lua_istable(L,1)){
        luaL_error(L, "Metro.__index: no self provided");
        goto error;
    }
    lua_getfield(L,1,"id"); // push id of metro onto stack. 1-based
    Metro_obj* obj = &metros[luaL_checkinteger(L,-1)-1]; // grab pointer to C struct. 0-based
    lua_pop(L,1); // remove pointer from stack

    if(!lua_isstring(L,2)){
        luaL_error(L, "Metro.__index: field doesn't exist");
        goto error;
    }
    const char* ix = luaL_checkstring(L,2);
    lua_settop(L,0);

    switch(ix[0]){
    case 's': // start / stop
        if(ix[3]=='r'){ // start
            lua_pushcfunction(L,l_metro_start);
        } else if(ix[3]=='p'){ // stop
            lua_pushcfunction(L,l_metro_stop);
        } else { // stage
            lua_pushinteger(L,obj->stage);
        }
        break;
    case 'i': // init
        lua_pushcfunction(L,l_metro_init);
        break;
    case 't':{ // time
        lua_pushnumber(L,obj->time);
        break;}
    case 'c': // count
        lua_pushinteger(L,obj->count);
        break;
    default:
        luaL_error(L, "Metro.__index: bad field");
        goto error;
    }
    return 1;

error:
    lua_settop(L, 0);
    return 0;
}

int l_metro__newindex( lua_State *L ){
    // metro[n].time = 1.1
    // metro[n].count = -1
    // metro[n].event = fn
    // args: self, index, value

    // pull userdata out of self
    if(!lua_istable(L,1)){
        luaL_error(L, "Metro.__newindex: no self provided");
        goto error;
    }
    lua_getfield(L,1,"id"); // push id of metro onto stack. 1-based
    Metro_obj* obj = &metros[luaL_checkinteger(L,-1)-1]; // grab pointer to C struct. 0-based
    lua_pop(L,1); // remove pointer from stack

    // decide which member is being updated
    if(!lua_isstring(L,2)){
        luaL_error(L, "Metro.__newindex: index doesn't exist");
        goto error;
    }
    const char* ix = luaL_checkstring(L,2);
    switch(ix[0]){
    case 't':{ // time
        float seconds = luaL_checknumber(L,3);
        if(seconds >= 0.0){
            obj->time = (seconds < 0.0005) ? 0.0005 : seconds;
            Metro_set_time(obj->id, obj->time);
        }
        break;}
    case 's': // stage
        obj->count = luaL_checkinteger(L,3);
        Metro_set_stage(obj->id, obj->count);
        break;
    case 'c': // count
        obj->count = luaL_checkinteger(L,3);
        Metro_set_count(obj->id, obj->count);
        break;
    case 'e': // event
        lua_rawset(L,1);
        break;
    default:
        luaL_error(L, "Metro.__newindex: bad index");
        goto error;
    }

error:
    lua_settop(L, 0);
    return 0;
}


// event handler

void L_queue_metro( int id, int state )
{
    event_t e = { .handler = L_handle_metro
                , .index.i = id
                , .data.i  = state
                };
    event_post(&e);
}

static void L_handle_metro( event_t* e )
{
    Metro_obj* obj = &metros[e->index.i];
    lua_getglobal(L, "metro"); // get metro table
    lua_pushinteger(L, obj->id+1); // lua is 1-based
    lua_gettable(L, 1); // leaves metro object at stack=2
    lua_getfield(L, -1, "event"); // push event fn onto TOS
    if(lua_isfunction(L,-1)){
        lua_pushinteger(L,e->data.i);
        lua_call(L,1,0); // 1 arg, no return
    }
}


// private definitions

static bool can_alloc(void){
    return available_metros > 0;
}

// naive search from the first stage as it's a v short list
static int allocate(void){
    for(int i=0; i<MAX_METROS; i++){
        if(metros[i].available){
            metros[i].available = false;
            available_metros--;
            return i;
        }
    }
    return -1; // failure
}

static void free_metro(int ix){
    Metro_obj* o = &metros[ix];
    o->available = true;
    set_defaults(ix);
}

static void set_defaults(int ix){
    Metro_obj* o = &metros[ix];
    o->time = 1.0;
    o->count = -1;
    o->stage = 0;
    o->id = ix;
}
