#include "metro.h"

#include <stm32f7xx.h>
#include <stdlib.h>            // malloc()

#include "../ll/timers.h"      // _Init() _Start() _Stop() _Set_Params()
#include "lualink.h"           // L_handle_metro()
#include "io.h"                // IO_handle_timer
#include "events.h"

typedef enum { METRO_STATUS_RUNNING
             , METRO_STATUS_STOPPED
} M_STATUS_t;

typedef struct{
    int        ix;       // TODO never used. metro index
    M_STATUS_t status;   // running/stopped status
    float      seconds;  // TODO never used. period in seconds
    int32_t    count;    // number of repeats. <0 is infinite
    int32_t    stage;    // number of repeateds.
} Metro_t;

Metro_t* metros;

// static declarations
static void Metro_bang( int ix );

// public definitions
int max_num_metros = 0;
void Metro_Init(void)
{
    max_num_metros = Timer_Init( Metro_bang );
    metros = malloc( sizeof(Metro_t) * max_num_metros );

    for( int i=0; i<max_num_metros; i++ ){
        metros[i].ix      = i;
        metros[i].status  = METRO_STATUS_STOPPED;
        metros[i].seconds = 1.0;
        metros[i].count   = -1;
        metros[i].stage   = 0;
    }
}

void Metro_start( int   ix
                , float seconds
                , int   count
                , int   stage
                )
{
    if( ix < 0
     || ix >= max_num_metros ){ printf("metro_start: bad index\n"); return; }

    Metro_t* t = &(metros[ix]);
    t->status = METRO_STATUS_RUNNING;
    t->count  = count;
    t->stage  = stage;

    Timer_Set_Params( ix, seconds );
    Timer_Start( ix );
}

// cancel all scheduled iterations
void Metro_stop( int ix )
{
    if( ix < 0
     || ix >= max_num_metros ){ printf("metro_stop: bad index\n"); return; }

    Metro_t* t = &(metros[ix]);
    if( t->status == METRO_STATUS_RUNNING ){
        t->status = METRO_STATUS_STOPPED;
        Timer_Stop( ix );
    }
}

// set period of metro
void Metro_set_time( int ix, float sec )
{
    if( ix < 0
     || ix >= max_num_metros ){ printf("metro_set_time: bad index\n"); return; }

    Timer_Set_Params( ix, sec ); // only using struct accessor
}

static void Metro_bang( int ix )
{
    // TODO confirm lua(1) makes a single tick
    if( ix < 2 ){
        IO_handle_timer( ix );
    } else {
        event_t e;
        e.type = E_metro;
        e.index = ix;
        e.data = metros[ix].stage;
        event_post(&e);
        //L_handle_metro( ix, metros[ix].stage );
    }
    metros[ix].stage++;
    //FIXME next line causes system not to load?
    if( metros[ix].stage == 0x7FFFFFFF ){ metros[ix].stage = 0x7FFFFFFE; } // overflow
    if( metros[ix].count >= 0 ){ // negative values are infinite
        if( metros[ix].stage > metros[ix].count ){
            Metro_stop(ix);
        }
    }
}
