#include "slews.h"

Slew_t slews[SLEW_CHANNELS];

// register a new destination
void S_toward( int        index
             , float      destination
             , float      ms
             , Shape_t    shape
             , Callback_t cb
             )
{
    if( index < 0 || index >= SLEW_CHANNELS ){ return }
    Slew_t* self = &slews[index]; // safe pointer

    // update destination
    self->dest   = destination;
    self->shape  = shape;
    self->action = cb;

    // update metadata
    self->last  = self->here; // need something smarter for shaping?
    const float t_scalar = SAMPLE_RATE / 1000.0;
    self->delta = (self->dest - self->here) / (ms * t_scalar);
}

// TODO: how to optimize this?
// compiler inlining probably goes pretty far
// inlining is likely faster than making it a fnptr & having to call
// thought to fnptr S_step_v() itself, but how to handle switching
// context at a breakpoint
void _S_isbreakpoint( Slew_t* self, float state )
{
    if( self->delta >= 0 ){
        if( state >= self->dest ){
            self->action;
        }
    } else {
        if( state < self->dest ){
            self->action;
        }
    }
}
float* S_step_v( Slew_t* self
               , float*  out
               , int     size
               )
{
    float* out2 = out;
    float* out3 = out;
    *out2++ = self->here + self->delta;
    _S_isbreakpoint( self, *out3 );
    for( int i=0; i<size; i++ ){
        *out2++ = *out3++ + self->delta;
        _S_isbreakpoint( self, *out3 );
    }
    self->here = *out3;
    return out;
}
