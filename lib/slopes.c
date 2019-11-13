#include "slopes.h"

#include <stdlib.h>
#include <math.h>

#include "stm32f7xx.h"

#include "wrBlocks.h" // for vectorized shapers


////////////////////////////////
// global vars
uint8_t slope_count = 0;
Slope_t* slopes = NULL;


////////////////////////////////
// private declarations

static float* step_v( Slope_t* self, float* out, int size );
static float* static_v( Slope_t* self, float* out, int size );
static float* motion_v( Slope_t* self, float* out, int size );
static float* breakpoint_v( Slope_t* self, float* out, int size );
static float* shaper_v( Slope_t* self, float* out, int size );
static float shaper( Slope_t* self, float out );

static float shaper_sin( float in );
static float shaper_log( float in );
static float shaper_exp( float in );
static float shaper_ease_in_back( float in );
static float shaper_ease_out_back( float in );
static float shaper_ease_out_rebound( float in );

typedef float (trig_t)(float in);
static float* shaper_v_sincos( Slope_t* self, trig_t fn, float* out, int size );
static float* shaper_v_sin( Slope_t* self, float* out, int size );
static float* shaper_v_cos( Slope_t* self, float* out, int size );
static float* shaper_v_exp( Slope_t* self, float* out, int size );
static float* shaper_v_log( Slope_t* self, float* out, int size );

////////////////////////////////
// public definitions

void S_init( int channels )
{
    slope_count = channels;
    slopes = malloc( sizeof( Slope_t ) * channels );
    if( !slopes ){ printf("slopes malloc failed\n"); return; }
    for( int j=0; j<SLOPE_CHANNELS; j++ ){
        slopes[j].index  = j;
        slopes[j].dest   = 0.0;
        slopes[j].last   = 0.0;
        slopes[j].shape  = SHAPE_Linear;
        slopes[j].action = NULL;

        slopes[j].here   = 0.0;
        slopes[j].delta  = 0.0;
        slopes[j].countdown = -1.0;
        slopes[j].scale = 0.0;
    }
}

Shape_t S_str_to_shape( const char* s )
{
    if( *s == 'l' ){
        if( s[1] == 'o' ){  return SHAPE_Log;
        } else {            return SHAPE_Linear; }
    } else if( *s == 's' ){ return SHAPE_Sine;
    } else if( *s == 'e' ){ return SHAPE_Expo;
    } else if( *s == 'o' ){ return SHAPE_Over;
    } else if( *s == 'u' ){ return SHAPE_Under;
    } else if( *s == 'r' ){ return SHAPE_Rebound;
    } else {                return SHAPE_Linear; //fallback to linear
    }
}

float S_get_state( int index )
{
    if( index < 0 || index >= SLOPE_CHANNELS ){ return 0.0; }
    Slope_t* self = &slopes[index]; // safe pointer
    return self->shaped;
}

// register a new destination
void S_toward( int        index
             , float      destination
             , float      ms
             , Shape_t    shape
             , Callback_t cb
             )
{
    if( index < 0 || index >= SLOPE_CHANNELS ){ return; }
    Slope_t* self = &slopes[index]; // safe pointer

    // save current output level as new starting point
    self->last   = self->shaped;

    // update destination
    self->dest   = destination;
    self->shape  = shape;
    self->action = cb;
    self->scale  = self->dest - self->last;

    // direct update & callback if ms = 0 (ie instant)
    if( ms <= 0.0 ){
        self->here = 1.0; // hard set to end of range
        self->countdown = -1.0; // inactive
        self->last = self->dest;
        self->shaped = self->dest;
    } else {
        float overflow = 0.0;
        if( self->countdown < 0.0 && self->countdown > -1023.0 ){
            overflow = -(self->countdown);
        }
        self->countdown = ms * SAMPLES_PER_MS; // samples until callback
        self->delta = 1.0 / self->countdown;
        self->here = 0.0; // start of slope
        if( overflow > 0.0 ){
            self->here += overflow * self->delta;
            self->countdown -= overflow;
            // TODO add protection against overshoot for very <1block slopes
        }
    }
}

float* S_step_v( int     index
               , float*  out
               , int     size
               )
{
    // turn index into pointer
    if( index < 0 || index >= SLOPE_CHANNELS ){ return out; }
    Slope_t* self = &slopes[index]; // safe pointer

    return step_v( self, out, size );
}



///////////////////////
// private defns

static float* step_v( Slope_t* self
                    , float*   out
                    , int      size
                    )
{
    if( self->countdown <= 0.0 ){ // at destination
        static_v( self, out, size );
    } else if( self->countdown > (float)size ){ // no edge case
        motion_v( self, out, size );
    } else {
        breakpoint_v( self, out, size );
    }
    return out;
}

static float* static_v( Slope_t* self, float* out, int size )
{
    float* out2 = out;
    for( int i=0; i<size; i++ ){
        *out2++ = self->here;
    }
    if( self->countdown > -1024.0 ){ // count overflow samples
        self->countdown -= (float)size;
    }
    return shaper_v( self, out, size );
}

static float* motion_v( Slope_t* self, float* out, int size )
{
    float* out2 = out;
    float* out3 = out;

    if( self->scale == 0.0 || self->delta == 0.0 ){ // delay only
        for( int i=0; i<size; i++ ){
            *out2++ = self->here;
        }
    } else {
        *out2++ = self->here + self->delta;
        for( int i=1; i<size; i++ ){
            *out2++ = *out3++ + self->delta;
        }
    }
    self->countdown -= (float)size;
    self->here = out[size-1];
    return shaper_v( self, out, size );
}

static float* breakpoint_v( Slope_t* self, float* out, int size )
{
    if( size <= 0 ){ return out; }

    self->here += self->delta; // no collision can happen on first samp

    self->countdown -= 1.0;
    if( self->countdown <= 0.0 ){
        self->here = 1.0; // clamp for overshoot
        if( self->action != NULL ){
            Callback_t act = self->action;
            self->action = NULL;
            (*act)(self->index);
            // side-affects: self->{dest, shape, action, countdown, delta, (here)}
        }
        if( self->action != NULL ){ // instant callback
            printf("FIXME: shouldn't happen on crow\n");
            *out++ = shaper( self, self->here );
            // 1. unwind self->countdown (ADD it to countdown)
            // 2. recalc current sample with new slope
            // 3. below call should be on out[0] and size
            return step_v( self, out, size-1 );
        } else { // slope complete, or queued response
            self->here  = 1.0;
            self->delta = 0.0;
            *out++ = shaper( self, self->here );
            return static_v( self, out, size-1 );
        }
    } else {
        *out++ = shaper( self, self->here );
        return breakpoint_v( self, out, size-1 ); // recursive call
    }
}


///////////////////////////////
// shapers

// vectors for optimized segments (assume: self->shape is constant)
static float* shaper_v( Slope_t* self, float* out, int size )
{
    switch( self->shape ){
        //case SHAPE_Sine:   shaper_v_sincos( self, &sinf, out, size ); break;
        //case SHAPE_Cosine: shaper_v_sincos( self, &cosf, out, size ); break;
        //case SHAPE_Log:    shaper_v_log( self, out, size ); break;
        //case SHAPE_Expo:   shaper_v_exp( self, out, size ); break;
        case SHAPE_Linear: break;
        default: { // if no vector, use single-sample
            float* out2 = out;
            for( int i=0; i<size; i++ ){
                *out2 = shaper( self, *out2 );
                out2++;
            }
            // shaper() cleans up self->shaped etc
            return out; }
    }
    // map to output range
    b_add(
       b_mul( out
            , self->scale
            , size )
         , self->last
         , size );
    // save last state
    self->shaped = out[size-1];
    return out;
}

// single sample for breakpoint segment
static float shaper( Slope_t* self, float out )
{
    switch( self->shape ){
        case SHAPE_Sine:    out = shaper_sin( out ); break;
        case SHAPE_Log:     out = shaper_log( out ); break;
        case SHAPE_Expo:    out = shaper_exp( out ); break;
        case SHAPE_Over:    out = shaper_ease_out_back( out ); break;
        case SHAPE_Under:   out = shaper_ease_in_back( out ); break;
        case SHAPE_Rebound: out = shaper_ease_out_rebound( out ); break;
        case SHAPE_Linear: default: break; // Linear falls through
    }
    // map to output range
    out = (out * self->scale) + self->last;
    // save last state
    self->shaped = out;
    return out;
}


//////////////////////////////
// single sample shapers
// all operate over a range of (0,1)

#define M_PI   (3.141592653589793) // 64bit compatible
#define M_PI_2 (M_PI/2.0)

// shapers normalized (0,1). from:
// http://probesys.blogspot.com/2011/10/useful-math-functions.html

static float shaper_sin( float in )
{
    return -0.5 * (cosf( M_PI * in ) - 1.0);
}

static float shaper_exp( float in )
{
    return 1.0 - powf(2.0, -10.0 * in);
}

static float shaper_log( float in )
{
    return powf(2.0, 10.0 * (in - 1.0));
}

static float shaper_ease_in_back( float in )
{
    return in * in * ((1.70158 + 1.0) * in - 1.70158);
}

static float shaper_ease_out_back( float in )
{
    float in_1 = in - 1.0;
    return in_1 * in_1 * ((1.70158 + 1.0) * in_1 + 1.70158) + 1.0;
}

static float shaper_ease_out_rebound( float in )
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

static float* shaper_v_sincos( Slope_t* self, trig_t fn, float* out, int size )
{
    float range = self->dest - self->last;
    float pi2range = M_PI_2/range;
    printf("sincos\n");

    return b_add(
                b_mul(
                    b_map( fn,
                        b_mul(
                            b_add( out
                                 , -self->last
                                 , size )
                             , pi2range
                             , size )
                         , size )
                     , range
                     , size )
                  , self->last
                  , size );
}

static float* shaper_v_exp( Slope_t* self, float* out, int size )
{
    printf("TODO shaper_v_exp\n");
    return out;
}

static float* shaper_v_log( Slope_t* self, float* out, int size )
{
    printf("TODO shaper_v_log\n");
    return out;
}
