#include "midi.h"

#include "../ll/midi_ll.h"
#include <stdio.h>

#include "lualink.h" // L_queue_midi()

typedef enum{ MIDI_NOTEOFF       = 0x80
            , MIDI_NOTEON        = 0x90
            , MIDI_AFTERTOUCH    = 0xA0
            , MIDI_CC            = 0xB0
            , MIDI_PATCHCHANGE   = 0xC0
            , MIDI_CH_PRESSURE   = 0xD0
            , MIDI_PITCHBEND     = 0xE0
            , MIDI_SYS           = 0xF0
} MIDI_Cmd_e;

typedef enum{ MIDI_SYSEX         = 0xF0
            , MIDI_MTC_QUARTER   = 0xF1
            , MIDI_SONG_POS      = 0xF2
            , MIDI_SONG_SEL      = 0xF3
            , MIDI_TUNE          = 0xF6
            , MIDI_SYSEX_END     = 0xF7
            , MIDI_CLOCK         = 0xF8
            , MIDI_START         = 0xFA
            , MIDI_CONTINUE      = 0xFB
            , MIDI_STOP          = 0xFC
            , MIDI_ACTIVE_SENSE  = 0xFE
            , MIDI_SYS_RESET     = 0xFF
} MIDI_Sys_e;


// private declarations
static uint8_t MIDI_rx_cmd( void );
static uint8_t MIDI_rx_data( uint8_t count );
static void event_complete( uint8_t* d );
void MIDI_Handle_Error( void );

static void (*midi_event)(uint8_t* data);

// public defns
void MIDI_Active( int state, void(*midi_e)(uint8_t*) )
{
    midi_event = midi_e;
    static int is_active = 0;
    if( state && !is_active ){
        MIDI_ll_Init( &MIDI_Handle_LL
                    , &MIDI_Handle_Error
                    );
        if( MIDI_rx_cmd() ){ printf("midi0\n"); }
        is_active = 1;
    } else if( !state && is_active ){
        MIDI_ll_DeInit();
        is_active = 0;
    } else if( is_active ){
        if( MIDI_rx_cmd() ){ printf("retry failed\n"); }
    } // else ignore
}

static uint8_t receiving_packet = 0;
static uint8_t MIDI_rx_cmd( void )
{
    receiving_packet = 0;
    return MIDI_ll_Rx( 0, 1);
}

static uint8_t MIDI_rx_data( uint8_t count )
{
    receiving_packet = 1;
    return MIDI_ll_Rx( 1, count);
}

int sysex_count = 0;
void MIDI_Handle_LL( uint8_t* buf )
{
    if( !receiving_packet ){ // this is a new midi message
        int count = MIDI_byte_count(buf[0]);
        if( count > 0 ){
            MIDI_rx_data( count );
        } else if( count == 0 ){ // sys
            if( buf[0] == MIDI_SYSEX ){
                receiving_packet = 1;
                MIDI_ll_Rx( ++sysex_count, 1 );
            } else { // system action
                event_complete(buf);
            }
        } else { // error retry
            if( MIDI_rx_cmd() ){ printf("midi2\n"); }
        }
    } else { // receiving data
        if( buf[0] == MIDI_SYSEX ){
            if( buf[sysex_count] == MIDI_SYSEX_END ){
                event_complete(buf);
            } else {
                MIDI_ll_Rx( ++sysex_count, 1 );
            }
        } else {
            event_complete(buf);
        }
    }
}
void MIDI_Handle_Error( void )
{
    MIDI_rx_cmd();
}

int MIDI_byte_count( uint8_t cmd_byte )
{
    switch(cmd_byte & 0xF0){
        case MIDI_NOTEOFF:     return 2;
        case MIDI_NOTEON:      return 2;
        case MIDI_AFTERTOUCH:  return 2;
        case MIDI_CC:          return 2;
        case MIDI_PATCHCHANGE: return 2;
        case MIDI_CH_PRESSURE: return 1;
        case MIDI_PITCHBEND:   return 2;
        case MIDI_SYS:         return 0;
        default:               return -1;
    }
}

static void event_complete( uint8_t* d )
{
    midi_event(d);
    if( MIDI_rx_cmd() ){ printf("midi3\n"); }
}
