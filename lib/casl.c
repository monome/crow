#include "casl.h"

// TODO should S_toward be passed as a fnptr? or is it too closely linked?
#include "slopes.h" // S_toward

#include <stdio.h>

#define TO_COUNT   8
#define SEQ_COUNT  8
#define SEQ_LENGTH 8
#define DYN_COUNT  8

typedef enum{ ToLiteral
            , ToRecur
} ToControl;

typedef union{
    float   f;
    int     dyn;
    Shape_t shape;
} ElemO;

typedef enum{ ElemT_Float
            , ElemT_Shape
            , ElemT_Dynamic
} ElemT;

typedef struct{
    ElemO obj;
    ElemT type;
} Elem;

typedef struct{
    Elem volts;
    Elem time;
    Elem shape;
    ToControl ctrl;
} To;

typedef struct{
    To* stage[SEQ_LENGTH];
    int length;
    int pc;
} Sequence;

// typedef struct{
//     // TODO
//     Sequence* retstk[SEQ_COUNT];
// } Tree;


// To*       tos[TO_COUNT];
// Sequence* seqs[SEQ_COUNT];
// Tree      tree;


static To tos[TO_COUNT];
static int to_ix = 0; // just counts up allocated To slots
static Sequence seq;

static Elem dynamics[DYN_COUNT];
static int dyn_ix = 0;

// update to use self instead of an index lookup

void casl_init( void )
{
    // TODO return allocated self
}




static void parse_table( lua_State* L );
void casl_describe( int index, lua_State* L )
{
    // reset description

    to_ix  = 0; // clear to's
    dyn_ix = 0; // clear dynamics

    // reset sequence
    seq.length = 0;
    seq.pc     = 0;

    printf("casl_describe\n");
    parse_table(L);
}

// suite of functions for unwrapping elements of Lua tables
static int ix_type( lua_State* L, int ix )
{
    lua_pushnumber(L, ix); // query first table elem
    lua_gettable(L, -2);  // get above key from table on stack
    int ix_type = lua_type(L, -1); // types from lua.h
    lua_pop(L, 1);
    return ix_type;
}
static const char ix_char( lua_State* L, int ix )
{
    lua_pushnumber(L, ix); // query first table elem
    lua_gettable(L, -2);  // get above key from table on stack
    const char ix_char = luaL_checkstring(L, -1)[0]; // types from lua.h
    lua_pop(L, 1);
    return ix_char;
}
static float ix_num( lua_State* L, int ix )
{
    lua_pushnumber(L, ix); // query first table elem
    lua_gettable(L, -2);  // get above key from table on stack
    float ix_num = luaL_checknumber(L, -1); // types from lua.h
    lua_pop(L, 1);
    return ix_num;
}
static int ix_int( lua_State* L, int ix )
{
    lua_pushnumber(L, ix); // query first table elem
    lua_gettable(L, -2);  // get above key from table on stack
    int ix_int = luaL_checkinteger(L, -1); // types from lua.h
    lua_pop(L, 1);
    return ix_int;
}

static void read_to( To* t, lua_State* L );
static void parse_table( lua_State* L )
{
    switch( ix_type(L, 1) ){ // types from lua.h

        case LUA_TSTRING: { // TO, RECUR
            To* t = &tos[to_ix]; to_ix++; // allocate a To*
            seq.stage[seq.length] = t; seq.length++; // store To* into the Sequence
            switch( ix_char(L, 1) ){
                case 'T': read_to(t, L); break; // standard To
                case 'R':{ t->ctrl = ToRecur; break; } // Recur ctrlflow
                default: printf("ERROR char not found\n"); break;
            }
            break;}

        case LUA_TTABLE:{ // NEST
            // TODO allocate a new Sequence*
            int seq_len = lua_rawlen(L, -1);
            printf("seq_len %i\n",seq_len);
            for( int i=1; i<=seq_len; i++ ){ // Lua is 1-based
                lua_pushnumber(L, i); // grab the next elem
                lua_gettable(L, -2); // push that inner-table to the stack
                parse_table(L); // RECUR
                lua_pop(L, 1); // pops inner-table
            }
            break;}

        default: printf("ERROR unhandled parse type\n"); break;
    }
}

static void capture_elem( Elem* e, lua_State* L, int ix );
static void read_to( To* t, lua_State* L )
{
    capture_elem(&(t->volts), L, 2);
    capture_elem(&(t->time), L, 3);

    // TODO capture
    lua_pushnumber( L, 4 ); // shape
    lua_gettable( L, -2 );
    t->shape.obj.shape = S_str_to_shape( luaL_checkstring(L, -1) );
    t->shape.type = ElemT_Shape;
    lua_pop( L, 1 );

    t->ctrl = ToLiteral;
}

// static void capture_elem( lua_State* L, int ix, Elem* e, ElemT* t )
// REFACTOR to have it return the Elem (and copy it) rather than passing a pointer
static void capture_elem( Elem* e, lua_State* L, int ix )
{
    switch( ix_type(L, ix) ){ // type of table elem
        case LUA_TNUMBER:{
            e->obj.f = ix_num(L, ix);
            e->type = ElemT_Float;
            break;}

        case LUA_TTABLE: // handle behavioural-types
            lua_pushnumber(L, ix); // push ix of To
            lua_gettable(L, -2);  // unwrap To[ix]
            switch( ix_char(L, 1) ){ // parse on first char at ix[1]
                case 'D':{
                    e->obj.dyn = ix_int(L, 2); // grab dyn_ix at ix[2]
                    e->type = ElemT_Dynamic;
                    break;}
                default: printf("ERROR composite To char not found\n"); break;
            }
            lua_pop(L, 1);
            break;
            
        default: printf("ERROR unknown To type\n"); break;
    }
}


///////////////////////////////
// Runtime

static void next_action( int index );
static ElemO resolve( Elem* e );

void casl_action( int index, int action )
{
    seq.pc = 0; // start at the beginning
    next_action( index );
}

static void next_action( int index )
{
    if( seq.pc < seq.length ){
        To* t = seq.stage[seq.pc]; seq.pc++;
        switch(t->ctrl){
            case ToLiteral:
                S_toward( index
                        , resolve(&t->volts).f
                        , resolve(&t->time).f * 1000.0
                        , resolve(&t->shape).shape
                        , &next_action // recur upon breakpoint
                        );
                break;

            case ToRecur:{
                // printf("rec\n");
                seq.pc = 0;
                next_action(index);
                return;}
        }
    } else { // sequence over
        // TODO check if the retstk has data so we can jump up
    }
}

// resolves behavioural types to a literal value
static ElemO resolve( Elem* e )
{
    // TODO add other cases for different behavioural types
    switch( e->type ){
        case ElemT_Dynamic: return resolve( &dynamics[e->obj.dyn] );
        default:            return e->obj;
    }
}


////////////////////////////////////
// Dynamic Variables

int casl_defdynamic( int index )
{
    int ix = dyn_ix; dyn_ix++;
    return ix;
}

void casl_setdynamic( int index, int dynamic_ix, float val )
{
    dynamics[dynamic_ix].obj.f = val;
    dynamics[dynamic_ix].type  = ElemT_Float; // FIXME support other types
}


/* operators
    math: + - * / %
        (add a b)
        (sub a b)
        (mul a b)
        (div a b)
        (mod a b)

    ctrl: loop held lock times (if while)
        (do ...)        ;; just sequences args once
        (loop ...)      ;; repeats args indefinitely
        (if p ...)      ;; executes args if predicate is true (checks at each step)
        (lock ...)      ;; sets the lock bit (against directives) then executes args
        (times n ...)   ;; executes all args n times (so (do) == (times 1))

    data: literal dynamic (can both be 'number' or 'shape')
        3
        54.2
        e    ;; char. act as enum for shapes
        &3   ;; dynamic index

    seqn: (table of data with behaviour)

    special ops:
        (in n)  ;; value of input[n]
        (rand)  ;; random value (use args?)

*/

/* data-types
    number: represents time & volts
    shape: an enum of available LUTs
*/

/* data-behaviours
    literal: a fixed value
    dynamic: a named value with an update method (eg. changable lfo time)
    sequin: a collection of data (of the same data-type) with a traversal-behaviour
        seq-behaviours: +n, drunk, random (index overflows, non-saturating)
    iterable: a number with a range and a iterating-behaviour
        iter-behaviours: +n, drunk-n, random
nb: both sequin & iterable are dynamic in nature, and can be updated if named
*/

/* directives
    ctrl: next restart release pause
    data: kset aset (update 'dynamic', k: at next breakpoint, a: immediate)
*/
