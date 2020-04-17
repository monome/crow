#pragma once

#include <stdint.h>

// need an Init() fn. send SR as an argument
#define SAMPLE_RATE 48000
#define iSAMPLE_RATE (1.0/(float)SAMPLE_RATE)
#define SAMPLES_PER_MS ((float)SAMPLE_RATE/1000.0)

typedef enum{ SHAPE_Linear
            , SHAPE_Sine
            , SHAPE_Log
            , SHAPE_Expo
            , SHAPE_Now
            , SHAPE_Wait
            , SHAPE_Over
            , SHAPE_Under
            , SHAPE_Rebound
} Shape_t;

typedef void (*Callback_t)(void* ptr);

typedef struct{
    int         index;
    // destination
    float       dest;
    float       last; // previous dest
    Shape_t     shape;
    Callback_t  action;
    // state
    // FIXME: rename to fade
    float       here;      // current interp (0,1)
    // FIXME: rename delta to inc
    float       delta;     // increment per sample
    float       countdown; // samples until breakpoint

    // pre-calcd
    float scale; // dest - last
    float shaped; // current shaped output voltage
} Slope_t;

// refactor for dynamic SAMPLE_RATE

Slope_t* S_init( void );
void S_deinit( Slope_t* self );

Shape_t S_str_to_shape( const char* s );

float S_get_state( Slope_t* self );
void S_toward( Slope_t*   self
             , float      destination
             , float      ms
             , Shape_t    shape
             , Callback_t cb
             );
float* Slope_v( Slope_t* self
              , float*   out
              , int      size
              );
