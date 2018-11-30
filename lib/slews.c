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
        slews[j].delta  = 0.0; //
        slews[j].scalar = SAMPLE_RATE / 1000.0; // TODO send SR as arg

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
        self->countdown = -1.0;        // inactive
        // FIXME. delay is broken without calling the next action directly
        //if( self->action != NULL ){ (*self->action)(index); } // call next step
    } else {
        self->countdown = ms * self->scalar; // samples until callback
        self->delta = (self->dest - self->here) / self->countdown;
    }
}

float* S_step_v( int     index
               , float*  out
               , int     size
               )
{
    // turn index into pointer
    if( index < 0 || index >= SLEW_CHANNELS ){ U_PrintLn("ix");return out; }
    Slew_t* self = &slews[index]; // safe pointer

    float* out2 = out;
    float* out3 = out;
    if( self->countdown <= 0.0 ){ // at destination
        for( int i=0; i<size; i++ ){
            *out2++ = self->dest;
        }
    } else if( self->countdown > (float)size ){ // no edge case
        if( self->delta == 0.0 ){ // delay only
            for( int i=0; i<size; i++ ){
                *out2++ = self->here;
            }
        } else {
            *out2++ = self->here + self->delta;
            for( int i=0; i<size; i++ ){
                *out2++ = *out3++ + self->delta;
            }
        }
        self->countdown -= (float)size;
    } else {
        // TODO add special case for delta == 0.0 ie delay
        int i = (int)self->countdown;
        float iRem = self->countdown - (float)i;
        float after_breakRem = 1.0 - iRem;
        int after_break = size - i - 1; // 1 is the iRem + after_breakRem

        *out2++ = self->here + self->delta;
        i--;
        for(; i>0; i-- ){
            *out2++ = *out3++ + self->delta;
        }
        if( iRem ){ *out2++ = *out3++ + self->delta; }
        // TODO compesnate for this amount of overshoot with the new trajectory
        // this means don't `+ self->delta` the last but just set to self->dest
        self->countdown = -after_breakRem;

        // BREAKPOINT!
        //TODO set self->action to NULL before calling
        if( self->action != NULL ){
            (*self->action)(index);
        }

        // NO ACTION (S_toward was not called by action)
        if( self->countdown <= 0.0 ){ // treat as 'arrived'
            self->here  = self->dest;
            self->delta = 0.0;
            for(; after_break>0; after_break-- ){
                *out2++ = self->here;
            }
            return out;
        }

        // NEXT STAGE
        // FIXME recursive function for 3 options takes i
        // TODO apply the partial step
        // FIXME currently assuming no breakpoint occurs here
        for( i=after_break; i>0; i-- ){
            *out2++ = *out3++ + self->delta;
        }
        self->countdown -= (float)after_break + after_breakRem;
    }
    self->here = *out3; // TODO overwrites delay() breakpoint
    return out;
}
