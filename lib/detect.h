#pragma once

#include <stm32f7xx.h>

typedef enum{ Detect_NONE
            , Detect_CHANGE
} Detect_mode_t;

typedef void (*Detect_callback_t)(int channel, float value);

typedef struct{
    float  threshold;
    float  hysteresis;
    int8_t direction;
} D_change_t;

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

// process fns
void Detect( Detect_t* self, float level );

Detect_mode_t Detect_str_to_mode( const char* str );
