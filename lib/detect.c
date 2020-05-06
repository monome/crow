#include "detect.h"

#include <stdlib.h>
#include <math.h>

uint8_t channel_count = 0;

Detect_t*  selves = NULL;


////////////////////////////////////////////////
// signal processor declarations

static void d_none( Detect_t* self, float level );
static void d_change( Detect_t* self, float level );
static void d_window( Detect_t* self, float level );
static void d_scale( Detect_t* self, float level );
static void d_volume( Detect_t* self, float level );


///////////////////////////////////////////
// init

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
        selves[j].win.lastWin = 0;
        selves[j].volume.vu = VU_init();
    }
}

void Detect_deinit( void )
{
    free(selves); selves = NULL;
}


/////////////////////////////////////////
// global helpers

Detect_t* Detect_ix_to_p( uint8_t index )
{
    if( index < 0 || index >= channel_count ){ return NULL; } // TODO error msg
    return &(selves[index]);
}

int8_t Detect_str_to_dir( const char* str )
{
    if( *str == 'r' ){ return 1; }
    else if( *str == 'f'){ return -1; }
    else{ return 0; } // default to 'both'
}


//////////////////////////////////////////
// mode configuration

void Detect_none( Detect_t* self )
{
    self->modefn = d_none;
}

void Detect_change( Detect_t*         self
                  , Detect_callback_t cb
                  , float             threshold
                  , float             hysteresis
                  , int8_t            direction
                  )
{
    self->modefn            = d_change;
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
    self->modefn        = d_scale;
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

void Detect_window( Detect_t*         self
                  , Detect_callback_t cb
                  , float*            windows
                  , int               wLen
                  , float             hysteresis
                  )
{
    printf("TODO need to sort the windows!\n");
    self->modefn         = d_window;
    self->action         = cb;
    self->win.wLen       = (wLen > WINDOW_MAX_COUNT) ? WINDOW_MAX_COUNT : wLen;
    self->win.hysteresis = hysteresis;
    for( int i=0; i<self->win.wLen; i++ ){
        self->win.windows[i] = *windows++;
    }
}

// this is the same as Stream
void Detect_volume( Detect_t*         self
                  , Detect_callback_t cb
                  , float             interval
                  )
{
    self->modefn         = d_volume;
    self->action         = cb;
    // SAMPLE_RATE * i / BLOCK_SIZE
    self->volume.blocks  = (int)((48000.0 * interval) / 32.0);
    if( self->volume.blocks <= 0 ){ self->volume.blocks = 1; }
    self->volume.countdown = self->volume.blocks;
}


//////////////////////////////////////////////
// signal processors
static void d_none( Detect_t* self, float level ){ return; }

static void d_change( Detect_t* self, float level )
{
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
}

static void d_window( Detect_t* self, float level )
{
    // search index containing 'level'
    int ix = 0;
        // TODO optimize: start from 'lastWin' rather than 0
    for(; ix<self->win.wLen; ix++ ){
        if( level < self->win.windows[ix] ){
            break;
        }
    }
    ix++; // 1-base the index so it can be passed with sign
    // compare the found win with 'lastWin'
    int lW = self->win.lastWin;
    if( ix != lW ){ // window has changed
        (*self->action)( self->channel
                       , (ix > lW) // sign of index determines direction
                            ? ix
                            : -ix
                       ); // callback!
        self->win.lastWin = ix; // save newly entered window
    }
}

static void d_scale( Detect_t* self, float level )
{
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
}

static void d_volume( Detect_t* self, float level )
{
    float v = VU_step( self->volume.vu, level );
    if( --self->volume.countdown <= 0 ){
        self->volume.countdown = self->volume.blocks; // reset counter
        (*self->action)( self->channel
                       , v
                       ); // callback!
    }
}
