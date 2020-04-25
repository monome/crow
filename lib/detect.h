#pragma once

#include <stm32f7xx.h>

#define SCALE_MAX_COUNT 16

typedef enum{ Detect_NONE
            , Detect_CHANGE
            , Detect_SCALE
} Detect_mode_t;

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
    uint8_t            channel;
    Detect_mode_t      mode;
    Detect_callback_t  action;

// mode specifics
  // Detect_change
    // params
    D_change_t    change;
    // state
    float         last;
    uint8_t       state;
  // Detect_scale
    D_scale_t     scale;
} Detect_t;

void Detect_init( int channels );

// mode functions
Detect_t* Detect_ix_to_p( uint8_t index );
int8_t Detect_str_to_dir( const char* str );

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

// process fns
void Detect( Detect_t* self, float level );
