#pragma once

#include <stm32f7xx.h>

#include "wrMeters.h"
#include "ftrack.h"

#define SCALE_MAX_COUNT 16
#define WINDOW_MAX_COUNT 16

typedef void (*Detect_void_callback_t)(uint8_t* data);
typedef void (*Detect_callback_t)(int channel, float value);

typedef struct{
    int blocks;
    int countdown;
} D_stream_t;

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
    float win;
    float hyst;
    // pre-calc for detection of next window
    float upper;
    float lower;
    // saved for remote access
    int lastIndex;
    int lastOct;
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
    int blocks;
    int countdown;
} D_volume_t;

typedef struct{
    float threshold;
    float hysteresis;
    float release;
    float envelope;
} D_peak_t;

typedef struct detect{
    uint8_t channel;
    void (*modefn)(struct detect* self, float level);
    Detect_callback_t action;

// state memory
    float      last;
    uint8_t    state; // for change/peak hysteresis

// mode specifics
    D_stream_t stream;
    D_change_t change;
    D_window_t win;
    D_scale_t  scale;

    VU_meter_t* vu; // vu metering for amplitude dtection
    D_volume_t  volume;
    D_peak_t    peak;
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
void Detect_stream( Detect_t*         self
                  , Detect_callback_t cb
                  , float             interval
                  );
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
void Detect_peak( Detect_t*         self
                , Detect_callback_t cb
                , float             threshold
                , float             hysteresis
                );
void Detect_freq( Detect_t*         self
                , Detect_callback_t cb
                , float             interval
                );
