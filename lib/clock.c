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
    uint32_t wakeup;
    int      coro_id;
    bool     running;
} clock_thread_t;

typedef struct{
    double wakeup;
    int    coro_id;
    bool   running;
} clock_thread_HD_t;

////////////////////////////////////
// global data

static int clock_count;
static clock_thread_t* clock_pool;
static clock_thread_HD_t internal; // for internal clocksource
static clock_source_t clock_source = CLOCK_SOURCE_INTERNAL;

static clock_reference_t reference;

static uint32_t last_tick = 0;


/////////////////////////////////////////////
// private declarations

static int find_idle(void);
static void clock_cancel( int index );

void clock_internal_run(uint32_t ms);

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
    clock_crow_init();
}


// TODO give clock it's own Timer to avoid this check & run in background
void clock_update(void)
{
    uint32_t time_now = HAL_GetTick();
    if( last_tick != time_now ){ // next tick
        last_tick = time_now;

        clock_internal_run(time_now);

        // TODO check for events
        // re-order the list whenever inserting new events
        // so the check code can just look at the front of the list
        // might need 2 separate (1 for sleep, 1 for sync)

        // for now we just check every entry!
        for( int i=0; i<clock_count; i++ ){
            if( clock_pool[i].running ){
                if( clock_pool[i].wakeup < time_now ){
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
        clock_pool[i].wakeup   = HAL_GetTick() + (uint32_t)(seconds * 1000.0);
        return true;
    }
    return false;
}

// some helpers to cleanup sync
static double zero_beat_time(void)
{
    return reference.last_beat_time - ((double)reference.beat_duration * reference.beat);
}
static double time_to_beats(double time_ms)
{
    return (time_ms - zero_beat_time()) / (double)reference.beat_duration;
}
static double beats_to_time(double beats)
{
    return zero_beat_time() + (beats * (double)reference.beat_duration);
}

bool clock_schedule_resume_sync( int coro_id, float beats )
{
    double current_time = clock_get_time_seconds(); // 1ms steps
    double current_beat = time_to_beats(current_time); // was 'this beat'

    double next_beat = (double)beats * floor(current_beat / (double)beats);
    double next_beat_time;
    do{
        next_beat += (double)beats;
        next_beat_time = beats_to_time(next_beat);
    } while( next_beat_time - current_time
           < (double)reference.beat_duration * (double)beats / (double)2000.0 );
        // i don't know why this value is 2000.0
        // seems like it should be 1000.0 to convert ms to seconds?
        // so i guess we have to divide by 2 for some reason...

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
    if( source >= 0 && source < CLOCK_SOURCE_LIST_LENGTH ){
        clock_source = source;
    }
}

float clock_get_time_beats(void)
{
    return (float)( time_to_beats( clock_get_time_seconds()));
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

void clock_cancel_coro_all( void )
{
    for( int i=0; i<clock_count; i++ ){
        clock_cancel(i);
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


/////////////////////////////////////////////////
// in clock_internal.h

static double internal_interval_seconds;
static double internal_beat;

void clock_internal_init(void)
{
    internal.running = false;
    clock_internal_set_tempo(120);
    clock_internal_start(0.0, true);
}

void clock_internal_set_tempo( float bpm )
{
    internal_interval_seconds = (double)60.0 / (double)bpm;
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
    internal.running = false; // actually stop the sync clock
    clock_stop_from( CLOCK_SOURCE_INTERNAL ); // user callback
}


/////////////////////////////////////
// private clock_internal

// note how we have to track the quantization error of the clock over cycles
// long-term precision is accurate to a double, while each clock pulse will
// be quantized to the tick *before* it's absolute position.
// this is important so that the beat division counter leads the userspace
// sync() calls & ensures they don't double-trigger.
void clock_internal_run(uint32_t ms)
{
    static double error = 0.0; // track ms error across clock pulses
    if( internal.running ){
        double time_now = ms;
        if( internal.wakeup < time_now ){
            internal_beat += 1;
            clock_update_reference_from( internal_beat
                                       , internal_interval_seconds
                                       , CLOCK_SOURCE_INTERNAL );
            internal.wakeup = time_now + internal_interval_seconds * (double)1000.0;
            error += 1+ floor(internal.wakeup) - internal.wakeup;
            if(error > (double)0.0){
                internal.wakeup -= (double)1.0;
                error--;
            }
        }
    }
}


/////////////////////////////////////////////////
// in clock_input.h

static bool clock_crow_last_time_set;
static int clock_crow_counter;
static double clock_crow_last_time;

#define DURATION_BUFFER_LENGTH 4

static float duration_buf[DURATION_BUFFER_LENGTH] = {0};
static uint8_t beat_duration_buf_pos = 0;
static uint8_t beat_duration_buf_len = 0;
static float mean_sum;
static float mean_scale;

static float crow_in_div = 4.0;


void clock_crow_init(void)
{
    clock_crow_counter = 0;
    clock_crow_last_time_set = false;
    mean_sum = 0;
}

// called by an event received on input
void clock_input_handler( int id, float freq )
{
    // this stub function just ignores the args
    clock_crow_handle_clock();
}
void clock_crow_handle_clock(void)
{
    float beat_duration;
    double current_time = clock_get_time_seconds();

    if( clock_crow_last_time_set == false ){
        clock_crow_last_time_set = true;
        clock_crow_last_time = current_time;
    } else {
        beat_duration = (float)(current_time - clock_crow_last_time)
                            * crow_in_div;
        if( beat_duration > 4.0 ){ // assume clock stopped
            clock_crow_last_time = current_time;
        } else {
            if( beat_duration_buf_len < DURATION_BUFFER_LENGTH ){
                beat_duration_buf_len++;
                mean_scale = 1.0 / beat_duration_buf_len;
            }

            float a = beat_duration * mean_scale;
            mean_sum = mean_sum + a;
            mean_sum = mean_sum - duration_buf[beat_duration_buf_pos];
            duration_buf[beat_duration_buf_pos] = a;
            beat_duration_buf_pos = (beat_duration_buf_pos + 1) % DURATION_BUFFER_LENGTH;

            clock_crow_counter++;
            clock_crow_last_time = current_time;

            double beat = clock_crow_counter / crow_in_div;
            clock_update_reference_from(beat, (double)mean_sum, CLOCK_SOURCE_CROW);
        }
    }
}

void clock_crow_in_div( float div )
{
    crow_in_div = 1.0/div;
}
