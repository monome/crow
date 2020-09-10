#pragma once

#include <stdbool.h>

// init
void Test_init( void );

// setters
void Test_power( bool status );
void Test_amps( int rail );
void Test_source( int option );
void Test_dest( int option );

// getters
bool Test_is_power( void );
