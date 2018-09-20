#include "slews.h"
#include "stm32f7xx.h"

Slew_t slews[SLEW_CHANNELS];

// register a new destination
void S_init( void )
{
    for( int j=0; j<SLEW_CHANNELS; j++ ){
        slews[j].dest   = 0.0;
        slews[j].shape  = SHAPE_Linear;
        slews[j].action = NULL;
        slews[j].here   = 0.0;
        slews[j].last   = 0.0;
        slews[j].delta  = 0.0;
    }
}

void S_toward( int        index
             , float      destination
             , float      ms
             , Shape_t    shape
             , Callback_t cb
             )
{
    if( index < 0 || index >= SLEW_CHANNELS ){ return; }
    Slew_t* self = &slews[index]; // safe pointer

    // update destination
    self->dest   = destination;
    self->shape  = shape;
    self->action = cb;

    // update metadata
    // TODO: make it time-based slopes, not convergence (support static delay)
    self->last  = self->here; // need something smarter for shaping?
    const float t_scalar = SAMPLE_RATE / 1000.0;
    self->delta = (self->dest - self->here) / (ms * t_scalar);
}

// TODO: how to optimize this?
// compiler inlining probably goes pretty far
// inlining is likely faster than making it a fnptr & having to call
// thought to fnptr S_step_v() itself, but how to handle switching
// context at a breakpoint
//
//
// update!!
//
// need to do the breakpoint based on *time* not destination
// this means we need to add a sense of time to the library
// but also means we don't have to do the complex edge cases
// and can calculate once per frame whether the breakpoint will
// be this frame.
//
// stretch: can also add overshoot compensation for sub-sample
// accuracy and time locking. similar approach to JF sawtooth
// lookup.
//
void _S_isbreakpoint( Slew_t* self, int id, float state )
{
    if( self->delta >= 0 ){
        if( state >= self->dest ){
            if( self->action != NULL ){ (*self->action)(id); }
        }
    } else {
        if( state < self->dest ){
            if( self->action != NULL ){ (*self->action)(id); }
        }
    }
}
float* S_step_v( int     index
               , float*  out
               , int     size
               )
{
    // turn index into pointer
    if( index < 0 || index >= SLEW_CHANNELS ){ return out; }
    Slew_t* self = &slews[index]; // safe pointer

    float* out2 = out;
    float* out3 = out;
    *out2++ = self->here + self->delta;
    _S_isbreakpoint( self, index, *out3 );
    for( int i=0; i<size; i++ ){
        *out2++ = *out3++ + self->delta;
        _S_isbreakpoint( self, index, *out3 );
    }
    self->here = *out3;
    return out;
}
