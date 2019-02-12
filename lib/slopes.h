#pragma once

#include <stdint.h>

// need an Init() fn. send SR as an argument
#define SAMPLE_RATE 48000

typedef enum{ SHAPE_Linear
            , SHAPE_Sine
            , SHAPE_Log
            , SHAPE_Expo
} Shape_t;

typedef void (*Callback_t)(int channel);

typedef struct{
    // destination
    float       dest;
    Shape_t     shape;
    Callback_t  action;
    // state
    float       here;      // current state
    float       delta;     // timestep per sample
    float       scalar;    // one ms in samples
    float       last;
    float       countdown; // samples until breakpoint
} Slope_t;

#define SLOPE_CHANNELS 4

// register a new destination
void S_init( void );
float S_get_state( int index );
void S_toward( int        index
             , float      destination
             , float      ms
             , Shape_t    shape
             , Callback_t cb
             );
float* S_step_v( int     index
               , float*  out
               , int     size
               );
