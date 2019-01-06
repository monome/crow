#include "detect.h"

#include <stdlib.h>

uint8_t channel_count = 0;
Detect_t** selves = NULL;

Detect_t** Detect_init( uint8_t channels )
{
    selves = malloc( sizeof( Detect_t ) * channels );
    // TODO null check
    channel_count = channels;
    for( uint8_t j=0; j<channels; j++ ){
        selves[j]->channel = j;
        Detect_mode( selves[j], Detect_none, NULL, NULL );
    }
    return selves;
}

void Detect_mode_ix( uint8_t           index
                   , Detect_mode_t     mode
                   , Detect_callback_t cb
                   , void*             data
                   )
{
    if( index < 0 || index >= channel_count ){ return; } // TODO error msg
    Detect_mode( selves[index]
               , mode
               , cb
               , data
               );
}
void Detect_mode( Detect_t*         self
                , Detect_mode_t     mode
                , Detect_callback_t cb
                , void*             data
                )
{
    self->mode   = mode;
    self->action = cb;
    switch( self->mode ){
        case Detect_none:
            break;

        case Detect_change:
            if( data ){
                D_change_t* c = (D_change_t*)data;
                self->change.threshold  = c->threshold;
                self->change.hysteresis = c->hysteresis;
                self->change.direction  = c->direction;
            } else { // defaults
                self->change.threshold  = 0.5;
                self->change.hysteresis = 0.1;
                self->change.direction  = 0;
            }
            // TODO need to reset state params?
            // can force update based on global struct members?
            break;

        default:
            break;
    }
}

void Detect( Detect_t* self, float level )
{
    switch( self->mode ){
        case Detect_none:
            break;

        case Detect_change:
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
        if( mode[1] == 'c' ){  return Detect_none; //In_scale;
        } else {               return Detect_none; }//In_stream;
    } else if( *mode == 'c' ){ return Detect_change; //In_change;
    } else if( *mode == 'w' ){ return Detect_none; //In_window;
    } else if( *mode == 'q' ){ return Detect_none; //In_quantize;
    } else if( *mode == 'j' ){ return Detect_none; //In_justintonation;
    } else {                   return Detect_none; //Detect_none;
    }
}
