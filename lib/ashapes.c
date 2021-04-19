#include "ashapes.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int ashaper_count = 0;
AShape_t* ashapers = NULL;

void AShaper_init( int channels )
{
    ashaper_count = channels;
    ashapers = malloc( sizeof( AShape_t ) * channels );
    if( !ashapers ){ printf("ashapers malloc failed\n"); return; }
    for( int j=0; j<ASHAPER_CHANNELS; j++ ){
        ashapers[j].index  = j;
        for( int d=0; d<MAX_DIV_LIST_LEN; d++ ){
            ashapers[j].divlist[d] = d; // ascending vals to 24
        }
        ashapers[j].dlLen   = 12;
        ashapers[j].modulo  = 12.0;
        ashapers[j].scaling = 1.0;
        ashapers[j].active  = false;
        ashapers[j].state   = 0.0;
    }
}

void AShaper_unset_scale( int index )
{
    if( index < 0 || index >= ASHAPER_CHANNELS ){ return; }
    AShape_t* self = &ashapers[index]; // safe pointer

    self->active = false;
}

void AShaper_set_scale( int    index
                      , float* divlist
                      , int    dlLen
                      , float  modulo
                      , float  scaling
                      )
{
    if( index < 0 || index >= ASHAPER_CHANNELS ){ return; }
    AShape_t* self = &ashapers[index]; // safe pointer

    self->active = true;

    self->dlLen = (dlLen > 24 ) ? 24 : dlLen;
    if( self->dlLen == 0 ){ // if empty list, assume chromatic
        self->dlLen = 1; // one step
        self->divlist[0] = 0.0; // set to unity

        self->modulo = 1.0;
        self->scaling = scaling / modulo;
    } else {
        for( int i=0; i<(self->dlLen); i++ ){
            self->divlist[i] = divlist[i];
        }
        self->modulo = modulo;
        self->scaling = scaling;
    }

    // private calculations //

    // pushes values up so capture window is centred on output window
    self->offset = 0.5 * self->scaling / self->modulo;
}

float AShaper_get_state( int index )
{
    if( index < 0 || index >= ASHAPER_CHANNELS ){ return 0.0; }
    AShape_t* self = &ashapers[index]; // safe pointer

    return self->state;
}

// TODO optimization
float* AShaper_v( int     index
                , float*  out
                , int     size
                )
{
    if( index < 0 || index >= ASHAPER_CHANNELS ){ return out; }
    AShape_t* self = &ashapers[index]; // safe pointer

    if( !self->active ){ // shaper inactive so just return
        self->state = out[size-1]; // save latest value
        return out;
    }

    float* out2 = out;
    for( int i=0; i<size; i++ ){
        float samp = *out2 + self->offset; // apply shift for centering and transpose

        float n_samp = samp/self->scaling; // samp normalized to [0,1.0)

        float divs = floorf(n_samp);
        float phase = n_samp - divs; // [0,1.0)

        int note = (int)(phase * self->dlLen); // map phase to num of note choices
        float note_map = self->divlist[note]; // apply lookup table
        note_map /= self->modulo; // remap via num of options

        *out2++ = self->scaling * (divs + note_map);
    }
    self->state = out[size-1]; // save last value
    return out;
}
