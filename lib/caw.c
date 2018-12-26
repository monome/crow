#include "caw.h"

#include <string.h>

#include "../usbd/usbd_main.h"
#include "../ll/debug_usart.h"

#define USB_RX_BUFFER 256
static char reader[USB_RX_BUFFER];
static int8_t pReader = 0;

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

static uint8_t _packet_complete( char last_char )
{
    if( last_char == '\0'
     || last_char == '\n'
     || last_char == '\r' ){ return 1; }
    return 0;
}

static uint8_t _find_cmd( char* str, uint32_t len )
{
    char* pStr = str;
    while( len-- ){ // FIXME should decrement first?
        if( *pStr++ == '^' ){
            if( *pStr++ == '^' ){
                if( *pStr == 'b' ){ return 1; }
                else if( *pStr == 's' ){ return 2; }
                else if( *pStr == 'e' ){ return 3; }
            }
        }
    }
    return 0;
}

C_cmd_t Caw_try_receive( void )
{
    // TODO add scanning for 'goto_bootloader' override command. return 2
    // TODO add start_flash_chunk command handling. return 3
    // TODO add end_flash_chunk command handling. return 4
    static C_cmd_t  mode = C_repl; // a tiny state machine
    static uint8_t* buf;
    static uint32_t len;

    if( USB_rx_dequeue( &buf, &len ) ){
        memcpy( &(reader[pReader])
              , (char*)buf
              , len
              );
        pReader += len;
        if( _packet_complete( reader[pReader-1] ) ){
            switch( _find_cmd( reader, pReader ) ){
                case 0: // no cmd. use repl. TODO should not set mode. just rtn
                    reader[pReader] = '\0';
                    pReader++;
                    return C_repl;
                case 1: // bootloader
                    return C_boot;
                case 2: // begin new script receive
                    Caw_get_read_len(); // clears buffer
                    return C_flashstart;
                case 3: // end script reception
                    Caw_get_read_len(); // clears buffer
                    return C_flashend;
                default: break;
            }
            return mode;
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
