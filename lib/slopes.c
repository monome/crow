#include "slopes.h"

#include <stdlib.h>

#include "stm32f7xx.h"

uint8_t slope_count = 0;
Slope_t* slopes = NULL;

// register a new destination
void S_init( int channels )
{
    slope_count = channels;
    slopes = malloc( sizeof( Slope_t ) * channels );
    if( !slopes ){ printf("slopes malloc failed\n"); return; }
    for( int j=0; j<SLOPE_CHANNELS; j++ ){
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
    if( index < 0 || index >= SLOPE_CHANNELS ){ return out; }
    Slope_t* self = &slopes[index]; // safe pointer

    //TODO wrap the below in a function
    // then apply it to a 'shaper' function that uses last & dest to calc
    // shaper function should have default shapes, but also capacity
    // to handle an array of quantize values either over the full range
    // or on a repeating octave

    float* out2 = out;
    float* out3 = out;
    if( self->countdown <= 0.0 ){ // at destination
        for( int i=0; i<size; i++ ){
            *out2++ = self->here;
        }
    } else if( self->countdown > (float)size ){ // no edge case
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
    } else {
        // TODO add special case for delta == 0.0 ie delay
        int i = (int)self->countdown;
        float iRem = self->countdown - (float)i;
        float after_breakRem = 1.0 - iRem;
        int after_break = size - i - 1; // 1 is the iRem + after_breakRem

        *out2++ = self->here + self->delta;
        i--;
        while(i--){
            *out2++ = *out3++ + self->delta;
        }
        *out2++ = self->dest;
        // TODO compesnate for this amount of overshoot with the new trajectory
        // this means don't `+ self->delta` the last but just set to self->dest
        self->countdown = -after_breakRem;

        // BREAKPOINT!
        //TODO set self->action to NULL before calling
        if( self->action != NULL ){
            (*self->action)(index);
            // FIXME to allow the action to be async we pretend we've 'arrived'
            // TODO a number of refinements can be added to improve accuracy:
            //  1 set a flag and count samples to compensate on next stage
            //  2 setup a continuation so next breakpoint can backtrack and fix samps
            //  3 request self->action 1 frame *before* breakpoint and enqueue the
            //      settings, so the vals are ready to be used locally
            self->here  = self->dest;
            self->delta = 0.0;
            for(; after_break>0; after_break-- ){
                *out2++ = self->here;
            }
            return out;
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
            //*out2++ = *out3++ + self->delta; // <- broken by event system
            *out2++ = self->here; // FIXME just clamping bc action hasn't applied yet
        }
        self->countdown -= (float)after_break + after_breakRem;
    }
    self->here = out[size-1]; // TODO overwrites delay() breakpoint
    return out;
}
