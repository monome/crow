#include "slopes.h"

#include <stdlib.h>
#include <math.h>

#include "stm32f7xx.h"


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
        slopes[j].shape  = SHAPE_Linear;
        slopes[j].action = NULL;
        slopes[j].here   = 0.0;
        slopes[j].delta  = 0.0; //
        slopes[j].scalar = SAMPLE_RATE / 1000.0; // TODO send SR as arg
        slopes[j].last   = 0.0;

        slopes[j].countdown = -1.0;
    }
}

float S_get_state( int index )
{
    if( index < 0 || index >= SLOPE_CHANNELS ){ return 0.0; }
    Slope_t* self = &slopes[index]; // safe pointer
    return self->here;
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

    // save current destination (for future shaping)
    self->last   = self->here;

    // update destination
    self->dest   = destination;
    self->shape  = shape;
    self->action = cb;

    // direct update & callback if ms = 0 (ie instant)
    if( ms <= 0.0 ){
        self->here      = destination; // hard set
        self->countdown = -1.0;        // inactive
        // FIXME. delay is broken without calling the next action directly
        //if( self->action != NULL ){ (*self->action)(index); } // call next step
    } else {
        float overflow = 0.0;
        if( self->countdown < 0.0 && self->countdown > -1023.0 ){
            overflow = self->countdown;
        }
        self->countdown = ms * self->scalar; // samples until callback
        self->delta = (self->dest - self->here) / self->countdown;
        if( overflow < 0.0 ){
            self->here -= overflow * self->delta;
            self->countdown += overflow;
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

    if( self->delta == 0.0 ){ // delay only
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
        self->here = self->dest; // clamp for overshoot
        if( self->action != NULL ){
            Callback_t act = self->action;
            self->action = NULL;
            (*act)(self->index);
            // side-affects: self->{dest, shape, action, countdown, delta, (here)}
        }
        if( self->action != NULL ){ // instant callback
            printf("FIXME: shouldn't happen on crow\n");
            out[0] = shaper( self, self->here );
            // 1. unwind self->countdown (ADD it to countdown)
            // 2. recalc current sample with new slope
            // 3. below call should be on out[0] and size
            return step_v( self, &out[1], size-1 );
        } else { // slope complete, or queued response
            self->here  = self->dest;
            self->delta = 0.0;
            out[0] = shaper( self, self->here );
            return static_v( self, &out[1], size-1 );
        }
    } else {
        out[0] = shaper( self, self->here );
        return breakpoint_v( self, &out[1], size-1 ); // recursive call
    }
}

static float* shaper_v( Slope_t* self, float* out, int size )
{
    float* out2 = out;
    for( int i=0; i<size; i++ ){
        *out2 = shaper( self, *out2 );
        out2++;
    }
    return out;
}

#define M_PI   (3.14159)
#define M_PI_2 (3.14159/2.0)
static float shaper( Slope_t* self, float out )
{
    switch( self->shape ){
        case SHAPE_Sine:   return shaper_sin( self, out );
        case SHAPE_Cosine: return shaper_cos( self, out );
        case SHAPE_Log:    return shaper_log( self, out );
        case SHAPE_Exp:    return shaper_exp( self, out );
        case SHAPE_Linear: return out;
    }
}


static float shaper_sin( Slope_t* self, float out )
{
    float range = self->dest - self->last;
    return self->last + range*sinf((out - self->last)*M_PI_2/(range));
}
static float shaper_cos( Slope_t* self, float out )
{
    float range = self->dest - self->last;
    return self->last + range*sinf((out - self->last)*M_PI_2/(range));
}





