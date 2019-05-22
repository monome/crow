#pragma once

#include <stdint.h>

void MIDI_Active( int state );
void MIDI_Handle_LL( uint8_t* buf );
int MIDI_byte_count( uint8_t cmd_byte );
