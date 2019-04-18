// events.c adapted from github.com/monome/libavr32

#include <stm32f7xx.h>
#include "events.h"
#include "lualink.h"


/// NOTE: if we are ever over-filling the event queue, we have problems.
/// making the event queue bigger not likely to solve the problems.
#define MAX_EVENTS   40

// macro for incrementing an index into a circular buffer.
#define INCR_EVENT_INDEX( x )  { if ( ++x == MAX_EVENTS ) x = 0; }

// get/put indexes into sysEvents[] array
volatile static int putIdx = 0;
volatile static int getIdx = 0;

// the system event queue is a circular array of event records
// NOTE be aware of event_t and MAX_EVENTS for RAM usage
volatile static event_t sysEvents[ MAX_EVENTS ];

void (*app_event_handlers[E_COUNT])(event_t *e) = { handler_none };

// initialize event handler
void events_init() {
	printf("\ninitializing event handler\n");
	int k;

	// set queue (circular list) to empty
	putIdx = 0;
	getIdx = 0;

	// zero out the event records
	for ( k = 0; k < MAX_EVENTS; k++ ) {
		sysEvents[ k ].type = 0;
		sysEvents[ k ].data = 0;
	}  

  // assign event handlers
  app_event_handlers[E_none] = &handler_none;
  app_event_handlers[E_metro] = &handler_metro;
  app_event_handlers[E_stream] = &handler_stream;
  app_event_handlers[E_change] = &handler_change;
  app_event_handlers[E_toward] = &handler_toward;
}

// get next event
// returns non-zero if an event was available
uint8_t event_next( event_t *e ) {
	uint8_t status;
	uint32_t old_primask = __get_PRIMASK();
	__disable_irq();

	// if pointers are equal, the queue is empty... don't allow idx's to wrap!
	if ( getIdx != putIdx ) {
		INCR_EVENT_INDEX( getIdx );
		e->type = sysEvents[ getIdx ].type;
		e->index = sysEvents[ getIdx ].index;
		e->data = sysEvents[ getIdx ].data;
		status = 1;
	} else {
		e->type  = 0xff;
    e->index = 0;
		e->data = 0;
		status = 0;
	}

	__set_PRIMASK( old_primask );

	return status;
}


// add event to queue, return success status
uint8_t event_post( event_t *e ) {
	//printf("posting event, type: %d\n",e->type);

	uint32_t old_primask = __get_PRIMASK();
	__disable_irq();

	uint8_t status = 0;

	// increment write idx, posbily wrapping
	int saveIndex = putIdx;
	INCR_EVENT_INDEX( putIdx );
	if ( putIdx != getIdx  ) {
		sysEvents[ putIdx ].type = e->type;
		sysEvents[ putIdx ].index = e->index;
		sysEvents[ putIdx ].data = e->data;
		status = 1;
	} else {
		// idx wrapped, so queue is full, restore idx
		putIdx = saveIndex;
	} 

	__set_PRIMASK( old_primask );

	//if (!status)
	//  printf("\r\n event queue full!");

	return status;
}


// EVENT HANDLERS

static void handler_none(event_t *e) {}

static void handler_metro(event_t *e) {
  //printf("metro event\n");
  L_handle_metro( e->index, e->data );
}

static void handler_stream(event_t *e) {
  //printf("stream event %f\n",e->data);
  L_handle_in_stream( e->index, e->data );
}

static void handler_change(event_t *e) {
  //printf("change event %f\n",e->data);
  L_handle_change( e->index, e->data );
}

static void handler_toward(event_t *e) {
  //printf("toward %d\n",e->index);
  L_handle_toward( e->index );
}
