#pragma once

#include <stdbool.h>

// lua libs for destructuring lua tables in C
#include "../submodules/lua/src/lua.h"
#include "../submodules/lua/src/lauxlib.h"
#include "../submodules/lua/src/lualib.h"

#include "slopes.h" // S_toward

#define TO_COUNT   16   // 28bytes
#define SEQ_COUNT  8    // 16bytes
#define SEQ_LENGTH 8    // 4bytes
#define DYN_COUNT  40   // 8bytes

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
    int     dyn; // index into the dynamics array
    uint16_t var[2]; // 2 indexes into dynamic table
    int     seq; // reference to a Sequence object
    Shape_t shape;
} ElemO; // 4bytes

typedef enum{ ElemT_Float
            , ElemT_Shape
            , ElemT_Dynamic
            , ElemT_Mutable
        // arithmetic ops
            , ElemT_Negate
            , ElemT_Add
            , ElemT_Sub
            , ElemT_Mul
            , ElemT_Div
            , ElemT_Mod
            , ElemT_Mutate
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

// TODO rename 'dynamics' to 'elements' and use it as a general abstraction (eg with To*)
typedef struct{
    To tos[TO_COUNT];
    int to_ix;

    Sequence* seq_current;
    Sequence seqs[SEQ_COUNT];
    int seq_ix;
    int seq_select;

    Elem dynamics[DYN_COUNT];
    int dyn_ix;

    bool holding;
    bool locked;
} Casl;

Casl* casl_init( int index );
void casl_describe( int index, lua_State* L );
void casl_action( int index, int action );

// dynamic vars
int casl_defdynamic( int index );
void casl_cleardynamics( int index );
void casl_setdynamic( int index, int dynamic_ix, float val );
float casl_getdynamic( int index, int dynamic_ix );
