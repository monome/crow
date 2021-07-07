#include "slopes.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "stm32f7xx.h"

#include "shapes.h"
#include "submodules/wrDsp/wrBlocks.h"


////////////////////////////////
// global vars

static uint8_t slope_count = 0;
static Slope_t* slopes = NULL;


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
    char ps = (char)*s;
    if( ps < 0x61 ){ ps += 0x20; } // convert upper to lowercase
    switch( ps ){ // match on first char unless necessary
        case 's': return SHAPE_Sine;
        case 'e': return SHAPE_Expo;
        case 'n': return SHAPE_Now;
        case 'w': return SHAPE_Wait;
        case 'o': return SHAPE_Over;
        case 'u': return SHAPE_Under;
        case 'r': return SHAPE_Rebound;
        case 'l': if( s[1]=='o' ){ return SHAPE_Log; } // else flows through
        default: return SHAPE_Linear; // unmatched
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

    // update destination
    self->dest   = destination;
    self->shape  = shape;
    self->action = cb;

    // direct update & callback if ms = 0 (ie instant)
    if( ms <= 0.0 ){
        self->last      = self->dest;
        self->shaped    = self->dest;
        self->scale     = 0.0;
        self->here      = 1.0; // hard set to end of range
        if(self->countdown > 0.0){
            // only happens when assynchronously updating S_toward
            self->countdown = -0.0; // inactive.
        }
    } else {
        // save current output level as new starting point
        self->last   = self->shaped;
        self->scale  = self->dest - self->last;
        float overflow = 0.0;
        if( self->countdown < 0.0 && self->countdown > -1023.0 ){
            overflow = -(self->countdown);
        }
        self->countdown = ms * SAMPLES_PER_MS; // samples until callback
        self->delta     = 1.0 / self->countdown;
        self->here      = 0.0; // start of slope
        if( overflow > 0.0 ){
            self->here += overflow * self->delta;
            self->countdown -= overflow;
            if( self->countdown <= 0.0 ){ // guard against overflow hitting callback
                printf("FIXME near immediate callback\n");
                // FIXME this should apply the destination & call self->action
                self->countdown = 0.00001; // force callback on next sample
                self->here = 1.0; // set to destination
            }
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
    } else { // WARN: requires size >= 1
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
        // TODO unroll overshoot and apply proportionally to the post-*act sample
        self->here = 1.0; // clamp for overshoot
        if( self->action != NULL ){
            Callback_t act = self->action;
            self->action = NULL;
            self->shaped = self->dest; // save real destination into shaped to actually reach it
            (*act)(self->index);
            // side-affects: self->{dest, shape, action, countdown, delta, (here)}
        }
        if( self->action != NULL ){ // instant callback
            *out++ = shaper( self, self->here );
            // 1. unwind self->countdown (ADD it to countdown)
            // 2. recalc current sample with new slope
            // 3. below call should be on out[0] and size
            if(size > 1){
                return step_v( self, out, size-1 );
            } else { // handle breakpoint on last sample of frame
                return out;
            }
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
        case SHAPE_Sine:    out = shapes_v_sin( out, size ); break;
        case SHAPE_Log:     out = shapes_v_log( out, size ); break;
        case SHAPE_Expo:    out = shapes_v_exp( out, size ); break;
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
        case SHAPE_Sine:    out = shapes_sin( out ); break;
        case SHAPE_Log:     out = shapes_log( out ); break;
        case SHAPE_Expo:    out = shapes_exp( out ); break;
        case SHAPE_Now:     out = shapes_step_now( out ); break;
        case SHAPE_Wait:    out = shapes_step_wait( out ); break;
        case SHAPE_Over:    out = shapes_ease_out_back( out ); break;
        case SHAPE_Under:   out = shapes_ease_in_back( out ); break;
        case SHAPE_Rebound: out = shapes_ease_out_rebound( out ); break;
        case SHAPE_Linear: default: break; // Linear falls through
    }
    // map to output range
    out = (out * self->scale) + self->last;
    // save last state
    self->shaped = out;
    return out;
}
