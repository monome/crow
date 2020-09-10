#include "test.h"

#include "ll/dout.h"
#include "ll/mux.h"


// global objects
static Dout* poweren;

// init
void Test_init( void )
{
    poweren = dout_init('c',2);
    Test_power( false );
}

// setters
void Test_power( bool status )
{
    dout_set( poweren, status );
}

// getters
bool Test_is_power( void )
{
    return dout_get( poweren );
}
