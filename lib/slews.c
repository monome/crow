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
        slews[j].last   = 0.0; //
        slews[j].delta  = 0.0; //

        slews[j].countdown = -1.0;
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

    // direct update & callback if ms = 0 (ie instant)
    if( ms <= 0.0 ){
        self->here      = destination; // hard set
        self->countdown = 0.0;         // force callback next sample
        //TODO just call the action directly!!
        //avoids off-by-one sample error
    } else {
        const float t_scalar = SAMPLE_RATE / 1000.0;
        self->countdown = ms * t_scalar; // samples until callback
        self->delta = (self->dest - self->here) / self->countdown;
    }
    self->last  = self->here;
}

// TODO: how to optimize this?
// stretch: can also add overshoot compensation for sub-sample
// accuracy and time locking. similar approach to JF sawtooth
// lookup.
static void _S_isbreakpoint( Slew_t* self, int id, float here )
{
    if( self->countdown <= 0.0 ){
        //TODO set self->action to NULL before calling
        if( self->action != NULL ){ (*self->action)(id); }
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
    if( self->countdown > (float)size ){ // no edge case
        *out2++ = self->here + self->delta;
        for( int i=0; i<size; i++ ){
            *out2++ = *out3++ + self->delta;
        }
        self->countdown -= (float)size;
    } else {
        float thiscd = (float)size - self->countdown;
        //TODO optimize loops as we can pre-calc exactly which
        //sample will have the edge case, so no need for a branch each time
        *out2++ = self->here + self->delta;
        _S_isbreakpoint( self, index, *out3 );
        for( int i=0; i<size; i++ ){
            *out2++ = *out3++ + self->delta;
            _S_isbreakpoint( self, index, *out3 );
        }
        self->countdown -= thiscd;
    }
    //TODO add elseif for countdown < -1.0 which means nothing set
    //so just fill buffer with a copy of here/dest.
    self->here = *out3; // TODO overwrites delay() breakpoint
    return out;
}
