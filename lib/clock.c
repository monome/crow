#include "clock.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lualink.h"
#include <stm32f7xx_hal.h> // HAL_GetTick


///////////////////////////////
// private types

typedef struct{
    // clock_update_reference is called every beat, so we're constantly updating beat & last_beat_time
    double beat; // set in clock_update_reference(beat,_). this is the refence beat (ie. count since start)
    double last_beat_time; // seconds_since_boot at last clock_update_reference() call
    double beat_duration_inverse; // inverse of beat_duration
    float  beat_duration; // seconds_per_beat (ie tempo)
} clock_reference_t;

typedef struct{
    double   beat_wakeup;
    uint32_t wakeup;
    int      coro_id;
    bool     running; // for clock.sleep
    bool     syncing; // for clock.sync (usees beat_wakeup)
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

// fp64 representation of beat count with a floating sub-beat count
static double precise_beat_now = 0;

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
        clock_pool[i].syncing = false;
    }

    clock_set_source( CLOCK_SOURCE_INTERNAL );
    clock_update_reference(0, 0.5); // set to zero beats, at 120bpm (0.5s/beat)

    // start clock sources
    clock_internal_init();
    clock_crow_init();
}

static double precision_beat_of_now(uint32_t time_now){
    double now_seconds = (double)time_now * (double)0.001; // ms to seconds
    double time_since_beat = now_seconds - reference.last_beat_time;
    double beat_fraction = time_since_beat * reference.beat_duration_inverse;
    return reference.beat + beat_fraction;
}

// This function must only be called when time_now changes!
// TIME SENSITIVE. this function is run every 1ms, so optimize it for speed.
void clock_update(uint32_t time_now)
{
    // increments the beat count if we've crossed into the next beat
    clock_internal_run(time_now);

    // calculate the fp64 beat count for .syncing checks
    precise_beat_now = precision_beat_of_now(time_now);

    // TODO check for events
    // re-order the list whenever inserting new events
    // so the check code can just look at the front of the list

    // for now we just check every entry!
    for( int i=0; i<clock_count; i++ ){
        if( clock_pool[i].running ){
            if( clock_pool[i].wakeup < time_now ){ // check for sleep events
                // event!
                // TODO pop from ordered list
                L_queue_clock_resume( clock_pool[i].coro_id );

                // i think this is coro cancel?
                clock_pool[i].running = false;
            }
        } else if( clock_pool[i].syncing ){ // check for sync events
            if( clock_pool[i].beat_wakeup < precise_beat_now ){
                L_queue_clock_resume( clock_pool[i].coro_id );

                // i think this is coro cancel?
                clock_pool[i].syncing = false;
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
    // returns the absolute system_time seconds where the 'zero' was
    // note: can probably be negative if clock has run a while, then tempo drastically decreased?
    return reference.last_beat_time - ((double)reference.beat_duration * reference.beat);
}
static double time_to_beats(double seconds)
{
    // expects seconds since system boot
    // returns which beat we're currently at. ie how beats since zero.
    return (seconds - zero_beat_time()) * reference.beat_duration_inverse;
}
static double beats_to_time(double beats)
{
    return zero_beat_time() + (beats * (double)reference.beat_duration);
}

// bool clock_schedule_resume_sync( int coro_id, float beats )
// {
//     double current_time = clock_get_time_seconds(); // seconds since system boot
//     double current_beat = time_to_beats(current_time); // current beat count, normalized to clock zero point

//     double next_beat = (double)beats * floor(current_beat / (double)beats);
//     // current_beat is always 0.002 more than the round-beat. next_beat is always 0 here
//     printf("%f %f %f\n", current_time, current_beat, next_beat);
//     double next_beat_time;
//     do{
//         next_beat += (double)beats;
//         printf(" %f %f\n", next_beat, beats);
//         next_beat_time = beats_to_time(next_beat);
//     } while( next_beat_time - current_time
//            // <= (double)reference.beat_duration * (double)beats / (double)2000.0 );
//            <= (double)reference.beat_duration * (double)beats / (double)1000.0 );
//         // i don't know why this value is 2000.0
//         // seems like it should be 1000.0 to convert ms to seconds?
//         // so i guess we have to divide by 2 for some reason...

//         // i just changed it to 1000.0 and everything seems correct?

//         // note: this fails when drastically increasing clock.tempo shortly after the beat at a slower tempo
//     printf(" %f\n", next_beat_time);

//     return clock_schedule_resume_sleep( coro_id
//                                       , (float)(next_beat_time - current_time) );
// }

bool clock_schedule_resume_sync( int coro_id, float beats ){
    int i;
    if( (i = find_idle()) >= 0 ){
        // TODO just mark the index as busy
            // the following data should only be in the ordered list
            // which should have a 'length' marker
        double dbeats = beats;

        // modulo sync time against base beat
        double awaken = floor(reference.beat / dbeats);
        awaken *= dbeats;
        awaken += dbeats;

        // check we haven't already passed it in the sub-beat & add another step if we have
        if(awaken <= precise_beat_now){
            awaken += dbeats;
        }

        clock_pool[i].coro_id     = coro_id;
        clock_pool[i].syncing     = true;
        clock_pool[i].beat_wakeup = awaken;
        return true;
    }
    return false;
}

// this function directly sleeps for an amount of beats (not sync'd to the beat)
bool clock_schedule_resume_beatsync( int coro_id, float beats ){
    return clock_schedule_resume_sleep(coro_id, beats * reference.beat_duration);
}

void clock_update_reference(double beats, double beat_duration)
{
    reference.beat_duration         = beat_duration;
    reference.beat_duration_inverse = (double)1.0 / (double)beat_duration; // for optimized precision_beat_of_now (called every ms)
    reference.last_beat_time        = clock_get_time_seconds(); // seconds since system boot
    reference.beat                  = beats;
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
    return 60.0 * (float)reference.beat_duration_inverse;
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
        if( !clock_pool[i].running && !clock_pool[i].syncing ){
            return i;
        }
    }
    return -1;
}

static void clock_cancel( int index )
{
    clock_pool[index].running = false;
    clock_pool[index].syncing = false;
    clock_pool[index].coro_id = -1;
}


/////////////////////////////////////////////////
// in clock_internal.h

static double internal_interval_seconds; // seconds. defined by internal BPM
static double internal_beat; // set by clock_internal_start(beat,_). count of beats since 0

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
