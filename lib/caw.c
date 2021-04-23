#include "caw.h"

#include <string.h>
#include <stdio.h>

#include "../usbd/usbd_main.h"

#define USB_RX_BUFFER 2048
static char reader[USB_RX_BUFFER];
static int16_t pReader = 0;

void Caw_Init( int timer_index )
{
    for( int i=0; i<USB_RX_BUFFER; i++ ){ reader[i] = 0; }
    USB_CDC_Init( timer_index );
}

void Caw_DeInit( void )
{
    USB_CDC_DeInit();
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
                switch( *pStr ){
                    case 'b': return C_boot;
                    case 's': return C_startupload;
                    case 'e': return C_endupload;
                    case 'w': return C_flashupload;
                    case 'c': return C_flashclear;
                    case 'r': return C_restart;
                    case 'p': return C_print;
                    case 'v': return C_version;
                    case 'i': return C_identity;
                    case 'k': return C_killlua;
                    case 'f': // fall through ->
                    case 'F': return C_loadFirst;
                }
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
    static uint8_t* buf;
    static uint32_t len;
    static uint8_t multiline = 0;

    C_cmd_t retcmd = C_none; // if nothing to dequeue do nothing

    if( USB_rx_dequeue_LOCK( &buf, &len ) ){
        retcmd = _find_cmd( (char*)buf, len ); // check for a system command
        if( retcmd != C_none ){ goto exit; } // sys command received, so skip ahead
        if( *buf == '\e' ){ // escape key
            pReader = 0;    // clear buffer
            retcmd = C_none;  // no action
            goto exit;
        }
        if( _is_multiline( (char*)buf ) ){
            multiline ^= 1;
            if(!multiline){
                retcmd = C_repl;
                goto exit;
            } else { // started new multiline
                if( len > 4 ){ // content follows the backticks
                    len -= 4;
                    buf = &buf[4];
                } else {
                    retcmd = C_none;
                    goto exit;
                }
            }
        }
        if( pReader + len > USB_RX_BUFFER ){ // overflow protection
            pReader = 0;
            Caw_send_luachunk("!chunk too long!");
            printf("!chunk too long!\n");
            //multiline = 0; // FIXME can we know whether this is high / low?
            retcmd = C_none;
            goto exit;
        }
    // receive code for repl/flash
        memcpy( &(reader[pReader])
              , (char*)buf
              , len
              );
        pReader += len;
        if( !multiline
         && _packet_complete( &reader[pReader-1] ) ){
            retcmd = C_repl;
            goto exit;
        }
    exit:
        USB_rx_dequeue_UNLOCK();
    }
    return retcmd;
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
