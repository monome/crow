#include "metro.h"

#include <stm32f7xx.h>
#include <stdlib.h>

#include "../ll/timers.h"
#include "../ll/debug_usart.h"

enum {
    METRO_STATUS_RUNNING,
    METRO_STATUS_STOPPED
};

typedef struct{
    int      ix;       // metro index
    int      status;   // running/stopped status
    float    seconds;  // period in seconds
    int      count;    // number of repeats. <0 is infinite
} Metro_t;

Metro_t* metros;

// static declarations
static void metro_bang( int ix );

// public definitions
int max_num_metros = 0;
void Metro_Init(void)
{
    max_num_metros = Timer_Init( metro_bang );
    metros = malloc( sizeof(Metro_t) * max_num_metros );

    // TODO add ll driver init?
    for( int i=0; i<max_num_metros; i++ ){
        metros[i].status  = METRO_STATUS_STOPPED;
        metros[i].seconds = 1.0;
        metros[i].count   = -1;
    }
}

void metro_start( int   ix
                , float seconds
                , int   count
                , int   stage
                )
{
    if( ix < 0
     || ix >= max_num_metros ){ U_PrintLn("metro_start: bad index"); return; }
    Metro_t* t = &metros[ix];

    t->count = count;

    Timer_Set_Params( ix, seconds, count ); // TODO currently discarding stage
    Timer_Start( ix );
}

// cancel all scheduled iterations
void metro_stop( int ix )
{
    if( ix < 0
     || ix >= max_num_metros ){ U_PrintLn("metro_stop: bad index"); return; }

    Timer_Stop( ix );
}

// set period of metro
void metro_set_time( int ix, float sec )
{
    if( ix < 0
     || ix >= max_num_metros ){ U_PrintLn("metro_set_time: bad index"); return; }

    Timer_Set_Params( ix, sec, metros[ix].count ); // only using struct accessor
}

static void metro_bang( int ix )
{
    U_Print("metro: "); U_PrintU8(ix);
    //union event_data *ev = event_data_new(EVENT_METRO);
    //ev->metro.id = t->ix;
    //ev->metro.stage = t->stage;
    //event_post(ev);
}
