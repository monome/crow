#pragma once

extern const int MAX_NUM_METROS;

void Metro_Init( int num_metros );

// control
void Metro_start( int ix );
void Metro_stop( int ix );
void Metro_stop_all( void );

// params
void Metro_set_time( int ix, float sec );
void Metro_set_count( int ix, int count );
void Metro_set_stage( int ix, int stage );
