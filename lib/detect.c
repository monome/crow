#include "detect.h"

#include <stdlib.h>
#include <math.h>

uint8_t channel_count = 0;

Detect_t*  selves = NULL;

void Detect_init( int channels )
{
    channel_count = channels;
    selves = malloc( sizeof ( Detect_t ) * channels );
    for( int j=0; j<channels; j++ ){
        selves[j].channel = j;
        selves[j].action  = NULL;
        selves[j].last    = 0.0;
        selves[j].state   = 0;
        Detect_none( &(selves[j]) );
    }
}

void Detect_deinit( void )
{
    free(selves); selves = NULL;
}

Detect_t* Detect_ix_to_p( uint8_t index )
{
    if( index < 0 || index >= channel_count ){ return NULL; } // TODO error msg
    return &(selves[index]);
}

void Detect_none( Detect_t* self )
{
    self->mode = Detect_NONE;
}

int8_t Detect_str_to_dir( const char* str )
{
    if( *str == 'r' ){ return 1; }
    else if( *str == 'f'){ return -1; }
    else{ return 0; } // default to 'both'
}
void Detect_change( Detect_t*         self
                  , Detect_callback_t cb
                  , float             threshold
                  , float             hysteresis
                  , int8_t            direction
                  )
{
    self->mode              = Detect_CHANGE;
    self->action            = cb;
    self->change.threshold  = threshold;
    self->change.hysteresis = hysteresis;
    self->change.direction  = direction;
    // TODO need to reset state params?
    // can force update based on global struct members?
}

void Detect_scale( Detect_t*         self
                 , Detect_callback_t cb
                 , float*            scale
                 , int               sLen
                 , float             divs
                 , float             scaling
                 )
{
    self->mode          = Detect_SCALE;
    self->action        = cb;
    self->scale.sLen    = (sLen > SCALE_MAX_COUNT) ? SCALE_MAX_COUNT : sLen;
    if( sLen == 0 ){ // assume chromatic
        self->scale.sLen = 1;
        self->scale.scale[0] = 0.0;
        self->scale.scaling = scaling / self->scale.divs; // scale to n-TET
        self->scale.divs    = 1.0; // force 1 div
    } else {
        for( int i=0; i<self->scale.sLen; i++ ){
            self->scale.scale[i] = *scale++;
        }
        self->scale.divs    = divs;
        self->scale.scaling = scaling;
    }
    self->scale.offset = 0.5 * self->scale.scaling / self->scale.divs;
    self->scale.lastNote = -100.0; // out of range, to force a new match
}

void Detect( Detect_t* self, float level )
{
    switch( self->mode ){
        case Detect_NONE:
            break;

        case Detect_CHANGE:
            if( self->state ){ // high to low
                if( level < (self->change.threshold - self->change.hysteresis) ){
                    self->state = 0;
                    if( self->change.direction != 1 ){ // not 'rising' only
                        (*self->action)( self->channel, (float)self->state );
                    }
                }
            } else { // low to high
                if( level > (self->change.threshold + self->change.hysteresis) ){
                    self->state = 1;
                    if( self->change.direction != -1 ){ // not 'falling' only
                        (*self->action)( self->channel, (float)self->state );
                    }
                }
            }
            break;

         case Detect_SCALE:
            // TODO add hysteresis to avoid jittering
            level += self->scale.offset;
            float n_level = level / self->scale.scaling;
            int octaves = (int)floorf(n_level);
            float phase = n_level - (float)octaves; // [0,1.0)
            float fix = phase * self->scale.sLen;
            int ix = (int)fix; // map phase to #scale

            if( ix      != self->scale.lastIndex
             || octaves != self->scale.lastOct
              ){ // new note detected
                float note = self->scale.scale[ix]; // apply LUT within octave
                float noteOct = note + (float)octaves*self->scale.divs;
                float volts = (note / self->scale.divs + (float)octaves)
                               * self->scale.scaling;
                self->scale.lastIndex = ix;
                self->scale.lastOct   = octaves;
                self->scale.lastNote  = noteOct;
                self->scale.lastVolts = volts;
                (*self->action)( self->channel, 0.0 ); // callback! 0.0 is ignored
            }
            break;

        default:
            break;
    }
}
