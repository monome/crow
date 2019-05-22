#pragma once

#include <stdint.h>

typedef struct{
    uint8_t bytes;
    uint8_t data[3]; // max data length
    char    name[];
} MIDI_Event_t;

void MIDI_Active( int state );
void MIDI_Handle_LL( uint8_t* buf );
MIDI_Event_t MIDI_Event( uint8_t* d );
