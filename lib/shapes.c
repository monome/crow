#include "shapes.h"

#include <math.h>
#include <stdio.h>

#include "submodules/wrDsp/wrBlocks.h" // for vectorized shapers

//////////////////////////////
// single sample shapers
// all operate over a range of (0,1)

#define M_PI   (3.141592653589793) // 64bit compatible
#define M_PI_2 (M_PI/2.0)

// shapers normalized (0,1). from:
// http://probesys.blogspot.com/2011/10/useful-math-functions.html

float shapes_sin( float in )
{
    return -0.5 * (cosf( M_PI * in ) - 1.0);
}

float shapes_exp( float in )
{
    return powf(2.0, 10.0 * (in - 1.0));
}

float shapes_log( float in )
{
    return 1.0 - powf(2.0, -10.0 * in);
}

float shapes_step_now( float in )
{
    return 1.0;
}

float shapes_step_wait( float in )
{
    // FIXME does this need a case for in approaching 1?
    return (in < 0.99999) ? 0.0 : 1.0;
}

float shapes_ease_in_back( float in )
{
    return in * in * (2.70158 * in - 1.70158);
}

float shapes_ease_out_back( float in )
{
    float in_1 = in - 1.0;
    return in_1 * in_1 * (2.70158 * in_1 + 1.70158) + 1.0;
}

float shapes_ease_out_rebound( float in )
{
    if( in < (1.0/2.75) ){
        return 7.5625 * in * in;
    } else if( in < (2.0/2.75) ){
        float in_c = in - 1.5/2.75;
        return 7.5625 * in_c * in_c + 0.75;
    } else if( in < (2.5/2.75) ){
        float in_c = in - 2.25/2.75;
        return 7.5625 * in_c * in_c + 0.9375;
    } else {
        float in_c = in - 2.625/2.75;
        return 7.5625 * in_c * in_c + 0.984375;
    }
}


////////////////////////////////
// vectorized shapers

float* shapes_v_sin( float* in, int size )
{
    return b_mul(
            b_add(
                b_map( cosf,
                    b_mul( in, M_PI, size )
                     , size )
                 , -1.0
                 , size )
                , -0.5
                , size );
}

static float pow2( float in ){ return powf(2.0,in); }

float* shapes_v_exp( float* in, int size )
{
    return b_map( pow2,
            b_mul(
                b_add( in, -1.0, size )
                 , 10.0
                 , size )
                , size );
}

float* shapes_v_log( float* in, int size )
{
    return b_sub(
            b_map( pow2,
                b_mul( in, -10.0, size )
                 , size )
                , 1.0
                , size );
}
