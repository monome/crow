#pragma once

#include <stm32f7xx.h>

#include "wrMeters.h"

#define SCALE_MAX_COUNT 16
#define WINDOW_MAX_COUNT 16

typedef void (*Detect_callback_t)(int channel, float value);

typedef struct{
    float  threshold;
    float  hysteresis;
    int8_t direction;
} D_change_t;

typedef struct{
    float scale[SCALE_MAX_COUNT];
    int   sLen;
    float divs;
    float scaling;
    // state / pre-computation
    float offset;
    int   lastIndex;
    int   lastOct;
    float lastNote;
    float lastVolts;
} D_scale_t;

typedef struct{
    float windows[WINDOW_MAX_COUNT];
    int   wLen;
    float hysteresis;
    int   lastWin;
} D_window_t;

typedef struct{
    VU_meter_t* vu;
    int         blocks;
    int         countdown;
} D_volume_t;

typedef struct detect{
    uint8_t            channel;
    void (*modefn)(struct detect* self, float level);

    Detect_callback_t  action;

// mode specifics
  // Detect_CHANGE
    // params
    D_change_t change;
    // state
    float      last;
    uint8_t    state;
  // Detect_WINDOW
    D_window_t win;
  // Detect_SCALE
    D_scale_t  scale;
  // Detect_VOLUME
    D_volume_t volume;
} Detect_t;

typedef void (*Detect_mode_fn_t)(Detect_t* self, float level);


////////////////////////////////////
// init

void Detect_init( int channels );
void Detect_deinit( void );


////////////////////////////////////
// global functions

Detect_t* Detect_ix_to_p( uint8_t index );
int8_t Detect_str_to_dir( const char* str );


/////////////////////////////////////
// mode configuration

void Detect_none( Detect_t* self );
void Detect_change( Detect_t*         self
                  , Detect_callback_t cb
                  , float             threshold
                  , float             hysteresis
                  , int8_t            direction
                  );
void Detect_scale( Detect_t*         self
                 , Detect_callback_t cb
                 , float*            scale
                 , int               sLen
                 , float             divs
                 , float             scaling
                 );
void Detect_window( Detect_t*         self
                  , Detect_callback_t cb
                  , float*            windows
                  , int               wLen
                  , float             hysteresis
                  );
void Detect_volume( Detect_t*         self
                  , Detect_callback_t cb
                  , float             interval
                  );
