#include "detect.h"

#include <stdlib.h>

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

void Detect( Detect_t* self, float level )
{
    switch( self->mode ){
        case Detect_NONE:
            break;

        case Detect_CHANGE:
            if( self->state ){
                if( level < (self->change.threshold - self->change.hysteresis) ){
                    self->state = 0;
                    (*self->action)( self->channel, (float)self->state );
                }
            } else {
                if( level > (self->change.threshold + self->change.hysteresis) ){
                    self->state = 1;
                    (*self->action)( self->channel, (float)self->state );
                }
            }
            break;

        default:
            break;
    }
}

Detect_mode_t Detect_str_to_mode( const char* mode )
{
    if( *mode == 's' ){
        if( mode[1] == 'c' ){  return Detect_NONE; //In_scale;
        } else {               return Detect_NONE; }//In_stream;
    } else if( *mode == 'c' ){ return Detect_CHANGE; //In_change;
    } else if( *mode == 'w' ){ return Detect_NONE; //In_window;
    } else if( *mode == 'q' ){ return Detect_NONE; //In_quantize;
    } else if( *mode == 'j' ){ return Detect_NONE; //In_justintonation;
    } else {                   return Detect_NONE; //Detect_NONE;
    }
}
