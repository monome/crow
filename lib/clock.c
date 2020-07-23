#include "clock.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lualink.h"
#include <stm32f7xx_hal.h> // HAL_GetTick


///////////////////////////////
// private types

typedef struct{
    double beat;
    double last_beat_time;
    float  beat_duration;
} clock_reference_t;

typedef struct{
    int  coro_id;
    bool running;
    int  wakeup;
} clock_thread_t;


////////////////////////////////////
// global data

static int clock_count;
static clock_thread_t* clock_pool;
static clock_thread_t internal; // for internal clocksource
static clock_source_t clock_source = CLOCK_SOURCE_INTERNAL;

static clock_reference_t reference;

static int last_tick = 0;

static double internal_interval_seconds;
static double internal_beat;

/////////////////////////////////////////////
// private declarations

static int find_idle(void);
static void clock_cancel( int index );
static void clock_cancel_all(void);

void clock_internal_run(void);

/////////////////////////////////////////////
// public defs

void clock_init( int max_clocks )
{
    clock_count = max_clocks;
    clock_pool = malloc( sizeof(clock_thread_t) * clock_count );
    for( int i=0; i<clock_count; i++ ){
        clock_pool[i].running = false;
    }

    clock_set_source( CLOCK_SOURCE_INTERNAL );
    clock_update_reference(0, 0.5);
    last_tick = HAL_GetTick();

    // start clock sources
    clock_internal_init();
}


// TODO give clock it's own Timer to avoid this check & run in background
void clock_update(void)
{
    int time_now = HAL_GetTick();
    if( last_tick != time_now ){ // next tick
        last_tick = time_now;

        // TODO check for events
        // re-order the list whenever inserting new events
        // so the check code can just look at the front of the list
        // might need 2 separate (1 for sleep, 1 for sync)

        // run a separate thread for the internal clock
        clock_internal_run();

        // for now we just check every entry!
        for( int i=0; i<clock_count; i++ ){
            if( clock_pool[i].running ){
                if( clock_pool[i].wakeup <= time_now ){
                    // event!
                    // TODO pop from ordered list
                    L_queue_clock_resume( clock_pool[i].coro_id );

                    // i think this is coro cancel?
                    clock_pool[i].running = false;
                }
            }
        }
    }
}

bool clock_schedule_resume_sleep( int coro_id, float seconds )
{
    int i;
    if( (i = find_idle()) >= 0 ){
        // TODO just mark the index as busy
            // the following data should only be in the ordered list
            // which should have a 'length' marker
        clock_pool[i].coro_id  = coro_id;
        clock_pool[i].running  = true;
        clock_pool[i].wakeup   = HAL_GetTick() + (int)(seconds * 1000.0);
        return true;
    }
    return false;
}

bool clock_schedule_resume_sync( int coro_id, float beats )
{
    double zero_beat_time;
    double this_beat;
    double next_beat;
    double next_beat_time;
    int next_beat_multiplier = 0;

    double current_time = clock_get_time_seconds();
    zero_beat_time = reference.last_beat_time
                        - ((double)reference.beat_duration * reference.beat);
    this_beat = (current_time - zero_beat_time) / (double)reference.beat_duration;

    do{
        next_beat_multiplier += 1;

        next_beat = (floor(this_beat / (double)beats) + next_beat_multiplier)
                        * (double)beats;
        next_beat_time = zero_beat_time + (next_beat * (double)reference.beat_duration);
    } while( next_beat_time - current_time
           < (double)reference.beat_duration * (double)beats / (double)2000.0 );

    return clock_schedule_resume_sleep( coro_id
                                      , (float)(next_beat_time - current_time) );
}

void clock_update_reference(double beats, double beat_duration)
{
    reference.beat_duration  = beat_duration;
    reference.last_beat_time = clock_get_time_seconds();
    reference.beat           = beats;
}

void clock_update_reference_from(double beats, double beat_duration, clock_source_t source)
{
    if( clock_source == source ){
        clock_update_reference( beats, beat_duration );
    }
}

void clock_start_from( clock_source_t source )
{
    if( clock_source == source ){
        L_queue_clock_start();
    }
}

void clock_stop_from( clock_source_t source )
{
    if( clock_source == source ){
        L_queue_clock_stop();
    }
}

void clock_set_source( clock_source_t source )
{
    clock_source = source;
    // TODO change clock source LL
}

float clock_get_time_beats(void)
{
    double current_time = clock_get_time_seconds();
    double zero_beat_time = reference.last_beat_time
                            - ((double)reference.beat_duration * reference.beat);
    return (float)(current_time - zero_beat_time) / reference.beat_duration;
}

double clock_get_time_seconds(void)
{
    return (double)HAL_GetTick() / (double)1000.0;
}

float clock_get_tempo(void)
{
    return 60.0 / reference.beat_duration;
}

void clock_cancel_coro( int coro_id )
{
    // TODO optimize search by looking through the 'active' list only
    for( int i=0; i<clock_count; i++ ){
        if( clock_pool[i].coro_id == coro_id ){
            clock_cancel(i);
        }
    }
}


////////////////////////////////////////////
// private defs

static int find_idle(void)
{
    // TODO optimize by starting search from last successful slot
    for( int i=0; i<clock_count; i++ ){
        if( !clock_pool[i].running ){
            return i;
        }
    }
    return -1;
}

static void clock_cancel( int index )
{
    clock_pool[index].running = false;
    clock_pool[index].coro_id = -1;
}

static void clock_cancel_all(void)
{
    for( int i=0; i<clock_count; i++ ){
        clock_cancel(i);
    }
}


/////////////////////////////////////////////////
// in clock_internal.h


void clock_internal_init(void)
{
    internal.running = false;
    clock_internal_set_tempo(120);
    clock_internal_start(0.0, true);
}

void clock_internal_set_tempo( float bpm )
{
    internal_interval_seconds = 60.0 / bpm;
    clock_internal_start( internal_beat, false );
}

void clock_internal_start( float new_beat, bool transport_start )
{
    internal_beat = new_beat;
    clock_update_reference_from( internal_beat
                               , internal_interval_seconds
                               , CLOCK_SOURCE_INTERNAL );

    if( transport_start ){
        clock_start_from( CLOCK_SOURCE_INTERNAL ); // user callback
    }
    internal.wakeup  = 0; // force event
    internal.running = true;
}

void clock_internal_stop(void)
{
    clock_stop_from( CLOCK_SOURCE_INTERNAL ); // user callback
}


/////////////////////////////////////
// private clock_internal

void clock_internal_run(void)
{
    if( internal.running ){
        double time_now = (double)HAL_GetTick();
        if( internal.wakeup <= (int)time_now ){
            internal_beat += (double)1.0;
            clock_update_reference_from( internal_beat
                                       , internal_interval_seconds
                                       , CLOCK_SOURCE_INTERNAL );
            int new = (int)(time_now + (internal_interval_seconds * (double)1000.0));
            internal.wakeup = new;
        }
    }
}
