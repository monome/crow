#include "casl.h"

// TODO should S_toward be passed as a fnptr? or is it too closely linked?
#include "slopes.h" // S_toward

#include <stdio.h>
void casl_describe( int index, char* description )
{
    printf("casl_describe[%i] %s\n", index, description);
    // parse the description into a tree of operations
    // compile a breakpoint handling function
    // will splice into the Callback_t of slopes.h


    S_toward( index
            , 1.0
            , 1000.0
            , SHAPE_Linear
            , NULL
            );
}

/* operators
    math: + - * /
    ctrl: loop held lock times (if while)
    data: literal dynamic (can both be 'number' or 'shape')
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
