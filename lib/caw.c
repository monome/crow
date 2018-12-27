#include "caw.h"

#include <string.h>

#include "../usbd/usbd_main.h"
#include "../ll/debug_usart.h"

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
    USB_tx_enqueue( buf, len );
}

// luachunk expects a \0 terminated string
void Caw_send_luachunk( char* text )
{
    USB_tx_enqueue( (uint8_t*)text, strlen(text) );
    uint8_t newline[] = "\n\r";
    USB_tx_enqueue( newline, 2 );
}

void Caw_send_luaerror( char* error_msg )
{
    uint8_t leader[] = "\\";
    USB_tx_enqueue( leader, 1 );
    USB_tx_enqueue( (uint8_t*)error_msg, strlen(error_msg) );
    uint8_t newline[] = "\n\r";
    USB_tx_enqueue( newline, 2 );
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
            default: break;
        }
        if( _is_multiline( (char*)buf ) ){
            multiline ^= 1;
            if(!multiline){ U_PrintLn(reader); }
            return (multiline) ? C_none : C_repl;
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
