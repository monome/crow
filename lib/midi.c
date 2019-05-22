#include "midi.h"

#include "../ll/midi_ll.h"

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

static int data_byte_count( MIDI_Cmd_e cmd );

// private declarations
static uint8_t MIDI_rx_cmd( void );
static uint8_t MIDI_rx_data( uint8_t count );
void L_queue_midi( uint8_t* d );


// public defns
int is_active = 0;
void MIDI_Active( int state )
{
    if( state && !is_active ){
        MIDI_ll_Init( &MIDI_Handle_LL );
        MIDI_rx_cmd();
    } else if( !state && is_active ){
        MIDI_ll_DeInit();
    }
}

uint8_t receiving_packet = 0;
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
        int count = data_byte_count(buf[0]);
        if( count > 0 ){ MIDI_rx_data( count ); }
        else if( count == 0 ){ // sys
            if( buf[0] == MIDI_SYSEX ){
                receiving_packet = 1;
                MIDI_ll_Rx( ++sysex_count, 1 );
            } else { // system action
                L_queue_midi(buf);
            }
        } else { // error retry
            MIDI_rx_cmd();
        }
    } else { // receiving data
        if( buf[0] == MIDI_SYSEX ){
            if( buf[sysex_count] == MIDI_SYSEX_END ){
                L_queue_midi(buf);
            } else {
                MIDI_ll_Rx( ++sysex_count, 1 );
            }
        } else {
            L_queue_midi(buf);
        }
    }
}

MIDI_Event_t MIDI_Event( uint8_t* d )
{
    MIDI_Event_t e; // FIXME create the struct
    return e; // return the struct by value (pass ownership to caller)

    // TODO lua callback
    switch( d[0] & 0xF0 ){ // ignore channel data while switching
        case MIDI_NOTEOFF:     break;
        case MIDI_NOTEON:      break;
        case MIDI_AFTERTOUCH:  break;
        case MIDI_CC:          break;
        case MIDI_PATCHCHANGE: break;
        case MIDI_CH_PRESSURE: break;
        case MIDI_PITCHBEND:   break;
        case MIDI_SYSEX:
            switch( d[0] ){
                case MIDI_SYSEX:
                    // use sysex_count in handler
                    sysex_count = 0;
                    break;
                case MIDI_MTC_QUARTER:   break;
                case MIDI_SONG_POS:      break;
                case MIDI_SONG_SEL:      break;
                case MIDI_TUNE:          break;
                case MIDI_CLOCK:         break;
                case MIDI_START:        break;
                case MIDI_CONTINUE:     break;
                case MIDI_STOP:         break;
                case MIDI_ACTIVE_SENSE:  break;
                case MIDI_SYS_RESET:     break;
                default: break;
            }
        default: break;
    }
    MIDI_rx_cmd(); // restart reception!
}

static int data_byte_count( MIDI_Cmd_e cmd )
{
    switch(cmd & 0xF0){
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
