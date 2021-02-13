#include "casl.h"

// TODO should S_toward be passed as a fnptr? or is it too closely linked?
#include "slopes.h" // S_toward

#include <stdio.h>

#define TO_COUNT   8
#define SEQ_COUNT  8
#define SEQ_LENGTH 8

typedef enum{ ControlNone
            , ControlRecur
} ControlFlow;

typedef struct{
    float volts;
    float time;
    int shape;
    ControlFlow ctrl;
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


static To tos[32];
static int to_ix = 0; // just counts up allocated To slots
static Sequence seq;


// update to use self instead of an index lookup

void casl_init( void )
{
    // TODO return allocated self
}




static void read_to( To* t, lua_State* L );
static void parse_table( lua_State* L );

void casl_describe( int index, lua_State* L )
{
    // reset description

    // deallocate all to's
    to_ix = 0;

    // reset sequence
    seq.length = 0;
    seq.pc     = 0;

    printf("casl_describe\n");
    parse_table(L);
}

static void parse_table( lua_State* L )
{
    printf("parse_table\n");
    lua_pushnumber(L, 1); // query first table elem
    lua_gettable(L, -2);  // get above key from table on stack
    int car_type = lua_type(L, -1); // types from lua.h

    switch( car_type ){ // types from lua.h

        case LUA_TSTRING: {
            const char s = luaL_checkstring(L, -1)[0]; // grab first char
            printf("string: %c\n",s);
            lua_pop(L, 1); // pop type
            switch(s){
                case 'T':{ // TO
                    printf("T\n");
                    To* t = &tos[to_ix]; to_ix++; // allocate a To*
                    read_to(t, L); // read it from Lua
                    seq.stage[seq.length] = t; seq.length++; // store To* into the Sequence
                    break;}

                case 'R': // RECUR
                    printf("R\n");
                    To* t = &tos[to_ix]; to_ix++; // allocate a To*
                    t->ctrl = ControlRecur;
                    seq.stage[seq.length] = t; seq.length++; // store To* into the Sequence
                    break;

                default:
                    printf("default\n");
                    break;
            }
            break;}

        case LUA_TTABLE:{ // NEST
            lua_pop(L, 1); // pop type
            // TODO allocate a new Sequence*
            int seq_len = lua_rawlen(L, -1);
            printf("seq_len %i\n",seq_len);
            for( int i=1; i<=seq_len; i++ ){ // Lua is 1-based
                printf("elem %i\n",i);
                lua_pushnumber(L, i); // grab the next elem
                lua_gettable(L, -2); // push that inner-table to the stack
                parse_table(L); // RECUR
                lua_pop(L, 1); // pops inner-table
            }
            break;}

        default:
            printf("default, type was %i\n",car_type);
            lua_pop(L, 1); // pop type
            break;
    }
}

static void read_to( To* t, lua_State* L )
{
    lua_pushnumber( L, 2 ); // volts
    lua_gettable( L, -2 );  // get above key from table on stack
    t->volts = luaL_checknumber( L, -1 ); // value is now on top of the stack
    lua_pop( L, 1 );                     // remove our introspected value

    lua_pushnumber( L, 3 ); // time
    lua_gettable( L, -2 );
    t->time = luaL_checknumber( L, -1 );
    lua_pop( L, 1 );

    lua_pushnumber( L, 4 ); // time
    lua_gettable( L, -2 );
    t->shape = S_str_to_shape( luaL_checkstring(L, -1) );
    lua_pop( L, 1 );

    t->ctrl = ControlNone;
}

static void next_action( int index );
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
            case ControlNone:
                S_toward( index
                        , t->volts
                        , t->time * 1000.0
                        , t->shape
                        , &next_action // recur upon breakpoint
                        );
                break;

            case ControlRecur:{
                printf("rec\n");
                seq.pc = 0;
                next_action(index);
                return;}
        }
    }
}










int casl_defdynamic( int index )
{
    // TODO return index for a newly allocated dynamic object
    return 0;
}

void casl_setdynamic( int index, int dynamic_ix, float val )
{
    // TODO set self->dynamic[dynamic_ix] = val
    // TODO support for types other than float
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



/* goal is to support commands in this order:

to(1, 0) -> _-
to(0, 1) -> -\
loop{ to(1, 0)
    , to(0, 1) } -> |\|\|\
loop{ to(1, 0)
    , to(0, dyn{time=1.0})} ->|\|\|-\|-\

*/
