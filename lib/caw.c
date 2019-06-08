#include "caw.h"

#include <string.h>

#include "../usbd/usbd_main.h"

#define USB_RX_BUFFER 1024
static char reader[USB_RX_BUFFER];
static int16_t pReader = 0;

uint8_t Caw_Init( void ) //TODO fnptr of callback )
{
    for( int i=0; i<USB_RX_BUFFER; i++ ){ reader[i] = 0; }
    USB_CDC_Init();
    // TODO set static void(*) which Caw_receive_callback() forward all info to
    // alt, we could just send a *buffer that we copy data into & check in scheduler
    return 0;
}

void Caw_send_raw( uint8_t* buf, uint32_t len )
{
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
    USB_tx_enqueue( buf, len );
__set_PRIMASK( old_primask );
}

// luachunk expects a \0 terminated string
void Caw_send_luachunk( char* text )
{
    const uint8_t newline[] = "\n\r";
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
    USB_tx_enqueue( (uint8_t*)text, strlen(text) );
    USB_tx_enqueue( (uint8_t*)newline, 2 );
__set_PRIMASK( old_primask );
}

void Caw_send_luaerror( char* error_msg )
{
    const uint8_t leader[] = "\\";
    const uint8_t newline[] = "\n\r";
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
    USB_tx_enqueue( (uint8_t*)leader, 1 );
    USB_tx_enqueue( (uint8_t*)error_msg, strlen(error_msg) );
    USB_tx_enqueue( (uint8_t*)newline, 2 );
__set_PRIMASK( old_primask );
}

void Caw_send_value( uint8_t type, float value )
{
    //
}

static C_cmd_t _find_cmd( char* str, uint32_t len )
{
    char* pStr = str;
    while( len-- ){ // FIXME should decrement first?
        if( *pStr++ == '^' ){
            if( *pStr++ == '^' ){
                if(      *pStr == 'b' ){ return C_boot; }
                else if( *pStr == 's' ){ return C_flashstart; }
                else if( *pStr == 'e' ){ return C_flashend; }
                else if( *pStr == 'c' ){ return C_flashclear; }
                else if( *pStr == 'r' ){ return C_restart; }
                else if( *pStr == 'p' ){ return C_print; }
                else if( *pStr == 'v' ){ return C_version; }
                else if( *pStr == 'i' ){ return C_identity; }
            }
        }
    }
    return C_none;
}

static uint8_t _is_multiline( char* first_char )
{
    if( *first_char++ == '`' ){
        if( *first_char++ == '`' ){
            if( *first_char == '`' ){ return 1; }
        }
    }
    return 0;
}

static uint8_t _packet_complete( char* last_char )
{
    if( *last_char == '\0'
     || *last_char == '\n'
     || *last_char == '\r' ){ return 1; }
    return 0;
}

C_cmd_t Caw_try_receive( void )
{
    // TODO add scanning for 'goto_bootloader' override command. return 2
    // TODO add start_flash_chunk command handling. return 3
    // TODO add end_flash_chunk command handling. return 4
    static uint8_t* buf;
    static uint32_t len;
    static uint8_t multiline = 0;

    if( USB_rx_dequeue( &buf, &len ) ){
    // check for a system command
        switch( _find_cmd( (char*)buf, len ) ){
            case C_boot:       return C_boot;
            case C_flashstart: return C_flashstart;
            case C_flashend:   return C_flashend;
            case C_flashclear: return C_flashclear;
            case C_restart:    return C_restart;
            case C_print:      return C_print;
            case C_version:    return C_version;
            case C_identity:   return C_identity;
            default: break;
        }
        if( *buf == '\e' ){ // escape key
            pReader = 0;    // clear buffer
            return C_none;  // no action
        }
        if( _is_multiline( (char*)buf ) ){
            multiline ^= 1;
            if(!multiline){
                return C_repl;
            } else { // started new multiline
                if( len > 4 ){ // content follows the backticks
                    len -= 4;
                    buf = &buf[4];
                } else { return C_none; }
            }
        }
        if( pReader + len > USB_RX_BUFFER ){ // overflow protection
            pReader = 0;
            Caw_send_luachunk("!chunk too long!");
            //multiline = 0; // FIXME can we know whether this is high / low?
            return C_none;
        }
    // receive code for repl/flash
        memcpy( &(reader[pReader])
              , (char*)buf
              , len
              );
        pReader += len;
        if( !multiline
         && _packet_complete( &reader[pReader-1] ) ){
            return C_repl;
        }
    }
    return C_none; // nothing to dequeue so always do nothing
}

char* Caw_get_read( void )
{
    return reader;
}

uint32_t Caw_get_read_len( void )
{
    uint32_t len = pReader;
    pReader = 0;
    return len;
}
