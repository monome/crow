#include "midi.h"

#include <stdio.h>
#include "caw.h"

static void MIDI_handler(uint8_t byte);

void MIDI_init(Midi* m, Uart* transmit){
    m->uart_tx = transmit;
}

void MIDI_transmit(Midi* m, uint8_t* bytes, int length){
    // FIXME this should be passed in as a protocol
    UART_send(m->uart_tx, bytes, length);
}

U8_Callback MIDI_get_callback(Midi* m){
    return &MIDI_handler;
}

static void MIDI_handler(uint8_t byte){
    Caw_printf("m 0x%x\n\r", byte);
}
