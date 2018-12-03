#pragma once

extern const int MAX_NUM_METROS;

void Metro_Init(void);

// create a metro at the specified index
// seconds < 0 == use previous period
void metro_start( int idx
                       , float seconds
                       , int count
                       , int stage
                       );

// cancel all scheduled iterations
void metro_stop( int idx );

// set period of metro
// NB: if the metro is running, its hard to say if new value will take effect
// on current period or next period
void metro_set_time( int idx, float sec );
