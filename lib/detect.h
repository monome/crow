#pragma once

#include <stm32f7xx.h>

typedef enum{ Detect_none
            , Detect_change
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
void Detect_ch_init( Detect_t* self, int index );
void Detect_mode_ix( uint8_t           index
                   , Detect_mode_t     mode
                   , Detect_callback_t cb
                   , void*             data
                   );
void Detect_mode( Detect_t*         self
                , Detect_mode_t     mode
                , Detect_callback_t cb
                , void*             data
                );
void Detect( Detect_t* self, float level );
Detect_mode_t Detect_str_to_mode( const char* str );
