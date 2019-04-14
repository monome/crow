// events.c adapted from github.com/monome/libavr32

#include <stm32f7xx.h>
#include "events.h"


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
		e->data = sysEvents[ getIdx ].data;
		status = 1;
	} else {
		e->type  = 0xff;
		e->data = 0;
		status = 0;
	}

	__set_PRIMASK( old_primask );

	return status;
}


// add event to queue, return success status
uint8_t event_post( event_t *e ) {
	// printf("\r\n posting event, type: ");
	// printf(e->type);

	uint32_t old_primask = __get_PRIMASK();
	__disable_irq();

	uint8_t status = 0;

	// increment write idx, posbily wrapping
	int saveIndex = putIdx;
	INCR_EVENT_INDEX( putIdx );
	if ( putIdx != getIdx  ) {
		sysEvents[ putIdx ].type = e->type;
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
