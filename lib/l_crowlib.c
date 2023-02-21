#include "l_crowlib.h"

#include <math.h>

#define L_CL_MIDDLEC 		(261.63f)
#define L_CL_MIDDLEC_INV 	(1.0f/L_CL_MIDDLEC)
#define L_CL_JIVOLT 		(1.0f/logf(2.f))


// called after crowlib lua file is loaded
// here we add any additional globals and such
void l_crowlib_init(lua_State* L){}


// Just Intonation calculators

static int justvolts(lua_State* L, float mul);

int l_crowlib_justvolts(lua_State* L){
	return justvolts(L, 1.f);
}

int l_crowlib_just12(lua_State *L){
	return justvolts(L, 12.f);
}

int l_crowlib_hztovolts(lua_State *L){
	// assume numbers, not tables
	float retval = 0.f;
	switch(lua_gettop(L)){
		case 1: // use default middleC reference
			// note we 
			retval = log2f(luaL_checknumber(L, 1) * L_CL_MIDDLEC_INV);
			break;
		case 2: // use provided reference
			retval = log2f(luaL_checknumber(L, 1)/luaL_checknumber(L, 2));
			break;
		default:
			lua_pushliteral(L, "need 1 or 2 args");
			lua_error(L);
			break;
	}
    lua_settop(L, 0);
	lua_pushnumber(L, retval);
	return 1;
}

static int justvolts(lua_State* L, float mul){
	// apply optional offset
	float offset = 0.f;
	switch(lua_gettop(L)){
		case 1: break;
		case 2: {offset = log2f(luaL_checknumber(L, 2))*mul;} break;
		default:
			lua_pushliteral(L, "need 1 or 2 args");
			lua_error(L);
			break;
	}

	// now do the conversion
	int nresults = 0;
	switch(lua_type(L, 1)){
		case LUA_TNUMBER:{
			float result = log2f(lua_tonumber(L, 1))*mul + offset;
			lua_settop(L, 0);
			lua_pushnumber(L, result);
			nresults = 1;
			break;}
		case LUA_TTABLE:{
			// get length of table to convert
			lua_len(L, 1);
			int telems = lua_tonumber(L, -1);
			lua_pop(L, 1);

			// build the new table in C (a copy)
			float newtab[telems+1]; // bottom element is unused
			for(int i=1; i<=telems; i++){
				lua_geti(L, 1, i);
				newtab[i] = log2f(luaL_checknumber(L, -1))*mul + offset;
				lua_pop(L, 1); // pops the number from the stack
			}

			// push the C table into the lua table
			lua_settop(L, 0);
			lua_createtable(L, telems, 0);
			for(int i=1; i<=telems; i++){
				lua_pushnumber(L, newtab[i]);
				lua_seti(L, 1, i);
			}
			nresults = 1;
			break;}
		default:
			lua_pushliteral(L, "unknown voltage type");
			lua_error(L);
			break;
	}
	return nresults;
}
