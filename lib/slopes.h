#pragma once

#include <stdint.h>

// need an Init() fn. send SR as an argument
#define SAMPLE_RATE 48000

typedef enum{ SHAPE_Linear
            , SHAPE_Sine
            , SHAPE_Cosine
            , SHAPE_Log
            , SHAPE_Expo
} Shape_t;

typedef void (*Callback_t)(int channel);

typedef struct{
    int         index;
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

// refactor for dynamic SLOPE_CHANNELS
// refactor for dynamic SAMPLE_RATE
// refactor to S_init returning pointers, but internally tracking indexes?

void S_init( int channels );

Shape_t S_str_to_shape( const char* s );

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
