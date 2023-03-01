#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum{ CLOCK_SOURCE_INTERNAL = 0
            , CLOCK_SOURCE_MIDI     = 1
            , CLOCK_SOURCE_LINK     = 2
            , CLOCK_SOURCE_CROW     = 3
            , CLOCK_SOURCE_LIST_LENGTH
} clock_source_t;

void clock_init( int max_clocks );

// MUST ONLY BE CALLED WHEN time_now CHANGES
// Designed to be called on a 1ms tick.
void clock_update(uint32_t time_now);

bool clock_schedule_resume_sleep( int coro_id, float seconds );
bool clock_schedule_resume_sync( int coro_id, float beats );
bool clock_schedule_resume_beatsync( int coro_id, float beats );
void clock_update_reference( double beats, double beat_duration );
void clock_update_reference_from( double beats, double beat_duration, clock_source_t source);
void clock_start_from( clock_source_t source );
void clock_stop_from( clock_source_t source );
void clock_set_source( clock_source_t source );

float clock_get_time_beats(void);
double clock_get_time_seconds(void);
float clock_get_tempo(void);

void clock_cancel_coro( int coro_id );
void clock_cancel_coro_all( void );


///////////////////////////////////
// internal

void clock_internal_init(void);
void clock_internal_set_tempo( float bpm );
void clock_internal_start( float new_beat, bool transport_start );
void clock_internal_stop(void);


///////////////////////////////////
// input

// TODO add arg to choose input channel
void clock_crow_init(void);
void clock_input_handler( int id, float freq ); // Called from Detect lib
void clock_crow_handle_clock(void);
void clock_crow_in_div( float div );
