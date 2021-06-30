#pragma once

#include <stdbool.h>

#define MAX_DIV_LIST_LEN 24

#define ASHAPER_CHANNELS 4

typedef struct{
    int    index;
    float  divlist[MAX_DIV_LIST_LEN];
    int    dlLen;
    float  modulo;
    float  scaling;
    float  offset;
    bool   active;
    float  state;
} AShape_t;

void AShaper_init( int channels );

void AShaper_unset_scale( int index );
void AShaper_set_scale( int    index
                      , float* divlist
                      , int    dlLen
                      , float  modulo
                      , float  scaling
                      );
float AShaper_get_state( int index );

float* AShaper_v( int     index
                , float*  out
                , int     size
                );
