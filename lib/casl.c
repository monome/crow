#include "casl.h"

// TODO should S_toward be passed as a fnptr? or is it too closely linked?
#include "slopes.h" // S_toward

#include <stdio.h>
#include <stdbool.h>

#define TO_COUNT   16 // 
#define SEQ_COUNT  8
#define SEQ_LENGTH 8
#define DYN_COUNT  8
#define ITER_COUNT 8

typedef enum{ ToLiteral
            , ToRecur
            , ToIf
            , ToEnter
            , ToHeld
            , ToWait
            , ToUnheld
            , ToLock
            , ToOpen
} ToControl;

typedef union{
    float   f;
    int     dyn;
    int     seq;
    int     iter;
    Shape_t shape;
} ElemO;

typedef enum{ ElemT_Float
            , ElemT_Shape
            , ElemT_Dynamic
            , ElemT_Iterable
} ElemT;

typedef struct{
    ElemO obj;
    ElemT type;
} Elem; // 8bytes

typedef struct{
    Elem a;
    Elem b;
    Elem c;
    ToControl ctrl;
} To; // 28bytes

typedef struct{
    To* stage[SEQ_LENGTH];
    int length;
    int pc;
    int parent; // seq_ix, like a tree 'parent' link
} Sequence;

static To tos[TO_COUNT];
static int to_ix = 0; // just counts up allocated To slots

static Sequence* seq_current;
static Sequence seqs[SEQ_COUNT];
static int seq_ix  = 0;  // counts up allocation of Sequence slots
static int seq_select = -1; // current 'parent'

static Elem dynamics[DYN_COUNT];
static int dyn_ix = 0;

static To iterables[ITER_COUNT];
static int iter_ix = 0;

static bool holding = false;
static bool locked = false;

// update to use self instead of an index lookup

void casl_init( void )
{
    // TODO return allocated self
}



static void seq_enter( void )
{
    if(seq_ix >= SEQ_COUNT){ printf("ERROR: no sequences left!\n"); return; }
    Sequence* s = &seqs[seq_ix];
    seq_current = s; // save as 'active' Sequence

    s->length = 0;
    s->pc     = 0;
    s->parent = seq_select;

    seq_select = seq_ix; // select the new Sequence
    printf("enter: %i\n",seq_select);
    seq_ix++; // marks the sequence as allocated
}

static void seq_exit( void )
{
    seq_select = seq_current->parent; // move up tree
    printf("exit: %i\n",seq_select);
    seq_current = &seqs[seq_select]; // save the new node
}


static void seq_append( To* t )
{
    Sequence* s = seq_current;
    if(s->length >= SEQ_LENGTH){ printf("ERROR: no stages left!\n"); }
    s->stage[s->length] = t; // append To* to end of Sequence
    s->length++;
}

static void parse_table( lua_State* L );
void casl_describe( int index, lua_State* L )
{
    // deallocate everything
    to_ix  = 0;
    dyn_ix = 0;
    seq_ix = 0;

    // enter first sequence
    seq_enter();

    printf("casl_describe\n");
    parse_table(L);
    // seq_exit()? // i think we want to start inside the first Seq anyway
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
static float ix_bool( lua_State* L, int ix )
{
    lua_pushnumber(L, ix); // query first table elem
    lua_gettable(L, -2);  // get above key from table on stack
    float ix_num = lua_toboolean(L, -1) ? 1.0 : 0.0;
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

static To* to_alloc( void )
{
    if(to_ix >= TO_COUNT){ return NULL; }
    To* t = &tos[to_ix]; // get next To
    to_ix++; // mark as allocated
    return t;
}

static void read_to( To* t, lua_State* L );
static void capture_elem( Elem* e, lua_State* L, int ix );
static void parse_table( lua_State* L )
{
    switch( ix_type(L, 1) ){ // types from lua.h

        case LUA_TSTRING: { // TO, RECUR
            To* t = to_alloc();
            if(t == NULL){ printf("ERROR: not enough To slots left\n"); return; }
            seq_append(t);
            switch( ix_char(L, 1) ){
                case 'T': read_to(t, L); break; // standard To
                case 'R':{ t->ctrl = ToRecur; break; } // Recur ctrlflow
                case 'I':{ // If ctrlflow
                    capture_elem(&(t->a), L, 2); // capture predicate from ix[2] to t->a
                    t->ctrl = ToIf;
                    break; }
                case 'H':{ t->ctrl = ToHeld; break; }
                case 'W':{ t->ctrl = ToWait; break; }
                case 'U':{ t->ctrl = ToUnheld; break; }
                case 'L':{ t->ctrl = ToLock; break; }
                case 'O':{ t->ctrl = ToOpen; break; }
                default: printf("ERROR char not found\n"); break;
            }
            break;}

        case LUA_TTABLE:{ // NEST
            To* t = to_alloc();
            seq_append(t);
            t->ctrl = ToEnter; // mark as entering a sub-Seq
            seq_enter();
            t->a.obj.seq = seq_select; // pass this To* to seq_enter, and do this line in there
            int seq_len = lua_rawlen(L, -1);
            printf("seq_len %i\n",seq_len);
            for( int i=1; i<=seq_len; i++ ){ // Lua is 1-based
                lua_pushnumber(L, i); // grab the next elem
                lua_gettable(L, -2); // push that inner-table to the stack
                parse_table(L); // RECUR
                lua_pop(L, 1); // pops inner-table
            }
            seq_exit();
            break;}

        default: printf("ERROR unhandled parse type\n"); break;
    }
}

static void read_to( To* t, lua_State* L )
{
    capture_elem(&(t->a), L, 2);
    capture_elem(&(t->b), L, 3);
    capture_elem(&(t->c), L, 4);
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

        case LUA_TBOOLEAN:{
            e->obj.f = ix_bool(L, ix);
            e->type = ElemT_Float;
            break;}

        case LUA_TSTRING:{
            char c = ix_char(L, ix);
            e->obj.shape = S_str_to_shape( &c );
            e->type = ElemT_Shape;
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

static To* seq_advance( void )
{
    Sequence* s = seq_current;
    To* t = NULL;
    if( s->pc < s->length ){ // stages remain
        t = s->stage[s->pc]; // get next stage
        s->pc++; // select following stage
    }
    return t;
}

// return true if there is an upval
static bool seq_up( void )
{
    if( seq_current->parent < 0 ){ return false; } // nothing left to do

    seq_current->pc = 0; // RESET PC so it runs more than once
    // TODO this will prob change when it comes to nesting conditionals?
    // TODO or at least when it comes to behavioural data

    seq_select = seq_current->parent;
    seq_current = &seqs[seq_select];
    return true;
}

static void seq_down( int s_ix )
{
    seq_select = s_ix;
    seq_current = &seqs[seq_select];
}

static void next_action( int index );
static bool find_control( ToControl ctrl, bool full_search );
static ElemO resolve( Elem* e );

void casl_action( int index, int action )
{
    if( locked ){ // can't apply action until unlocked
        if( action == 2 ){ locked = false; } // 'unlock' message received
        return; // doesn't trigger action
    }
    if( action == 1){ // restart sequence
        seq_current = &seqs[0]; // first sequence
        seq_current->pc = 0;   // first step
        holding = false;
        locked = false;
    } else if( action == 0 && holding ){ // goto release if held
        if( find_control(ToUnheld, false) ){
            holding = false;
        } else {
            printf("couldn't find ToWait. restarting\n");
            casl_action(index, 1);
            return;
        }
    } else {
        printf("do nothing\n");
        return;
    }
    next_action( index );
}

static void next_action( int index )
{
    To* t = seq_advance();
    if(t){ // To is valid
        switch(t->ctrl){
            case ToLiteral:{
                float ms = resolve(&t->b).f * 1000.0;
                S_toward( index
                        , resolve(&t->a).f
                        , ms
                        , resolve(&t->c).shape
                        , &next_action // recur upon breakpoint
                        );
                if(ms <= 0.0){ next_action(index); } // RECUR because the action was instant
                break;}

            case ToRecur:{
                seq_current->pc = 0;
                next_action(index);
                return;}

            case ToIf:{
                if( resolve(&t->a).f > 0.0 ){ next_action(index); } // pred is true
                else if( seq_up() ){ next_action(index); } // step up one level
                break;}

            case ToEnter:{
                seq_down(t->a.obj.seq);
                next_action(index);
                break;}

            case ToHeld:{ holding = true; break;}

            case ToWait: break; // do nothing. awaits next_action

            case ToUnheld:{
                printf("Unheld. i don't think this executes.\n");
                holding = false; // unmark held
                break;}

            case ToLock:{ locked = true; break;}
            case ToOpen:{ locked = false; break;}
        }
    } else if( seq_up() ){ // To invalid. Jump up the retstk
        next_action(index); // recur
    }
}

static bool find_control( ToControl ctrl, bool full_search )
{
    To* t = seq_advance();
    if(t){ // To is valid
        if(t->ctrl == ctrl){ return true; } // FOUND IT
        switch(t->ctrl){ // else handle navigation
            case ToEnter:
                if(full_search){ seq_down(t->a.obj.seq); }
                return find_control(ctrl, full_search);
            case ToIf:
                if( !full_search ){ seq_up(); } // skip If contents
                // FALLS THROUGH
            default:
                return find_control(ctrl, full_search);
        }
    } else if( seq_up() ){ // end of list. Jump up the retstk
        return find_control(ctrl, full_search); // recur
    }
    return false;
}

static Elem* iter_apply( ElemO o )
{
    To* i = &iterables[o.iter];
    i->a.obj.f -= i->b.obj.f;
    return &i->a;
}

static void iter_reset( ElemO o )
{
    To* i = &iterables[o.iter];
    i->a.obj.f = i->c.obj.f;
}

// resolves behavioural types to a literal value
static ElemO resolve( Elem* e )
{
    switch( e->type ){
        case ElemT_Dynamic: return resolve( &dynamics[e->obj.dyn] );
        case ElemT_Iterable: return resolve( iter_apply( e->obj ));
        default: return e->obj;
    }
}


////////////////////////////////////
// Dynamic Variables
// allows concurrent access of registered vars from C and Lua

int casl_defdynamic( int index )
{
    if(dyn_ix >= DYN_COUNT){ printf("ERROR: nno dynamic slots remain\n"); return -1; }
    int ix = dyn_ix; dyn_ix++;
    return ix;
}

void casl_setdynamic( int index, int dynamic_ix, float val )
{
    dynamics[dynamic_ix].obj.f = val;
    dynamics[dynamic_ix].type  = ElemT_Float; // FIXME support other types
}

float casl_getdynamic( int index, int dynamic_ix )
{
    switch(dynamics[dynamic_ix].type){
        case ElemT_Float:
            return dynamics[dynamic_ix].obj.f;
        default: printf("getdynamic! wrong type\n"); return 0.0;
    }
}


/* operators
    math: + - * / %
        (add a b)
        (sub a b)
        (mul a b)
        (div a b)
        (mod a b)

    ctrl: loop held lock times (if while)
        (do ...)        ;; sequences args once
        (loop ...)      ;; repeats args indefinitely
        (if p ...)      ;; executes args if predicate is true (checks at each step)
        (while p ...)   ;; repeats until predicate is false
        (held ...)      ;; like (if) but with 'attack' / 'release' directive support
        (lock ...)      ;; disables directives while executing args. can release with `unlock` directive
        (times n ...)   ;; executes all args n times (so (do) == (times 1))

    data: literal dynamic (can both be 'number' or 'shape')
        3
        54.2
        e    ;; char. act as enum for shapes
        &3   ;; dynamic index

    seqn: (table of data with behaviour)
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
    ctrl: restart release
    data: kset aset (update 'dynamic', k: at next breakpoint, a: immediate)
*/
