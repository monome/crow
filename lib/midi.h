#pragma once

#include <stdint.h>
#include "../ll/uart.h" // this requirement should be dropped by replacing Uart* with a "protocol" of pointer + operations

typedef struct{
    Uart* uart_tx;
} Midi;

void MIDI_init(Midi* m, Uart* transmit);

void MIDI_transmit(Midi* m, uint8_t* bytes, int length);

U8_Callback MIDI_get_callback(Midi* m);
