#include "caw.h"

#include <string.h>

#include "../usbd/usbd_main.h"

#define USB_RX_BUFFER 2048
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
                else if( *pStr == 'k' ){ return C_killlua; }
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

    C_cmd_t retcmd = C_none; // if nothing to dequeue do nothing

// FIXME
// do we know that the buf & len haven't been overwritten? can we check?
// we're assuming that a new CDC_Itf_Receive interrupt won't occur before below

    if( USB_rx_dequeue( &buf, &len ) ){
        switch( _find_cmd( (char*)buf, len ) ){ // check for a system command
            case C_boot:       retcmd = C_boot; goto exit;
            case C_flashstart: retcmd = C_flashstart; goto exit;
            case C_flashend:   retcmd = C_flashend; goto exit;
            case C_flashclear: retcmd = C_flashclear; goto exit;
            case C_restart:    retcmd = C_restart; goto exit;
            case C_print:      retcmd = C_print; goto exit;
            case C_version:    retcmd = C_version; goto exit;
            case C_identity:   retcmd = C_identity; goto exit;
            case C_killlua:    retcmd = C_killlua; goto exit;
            default: break;
        }
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
        if( USB_rx_dequeue( &buf, &len ) ){
            // if this happens it means the buffer is not being dealt with in time
            printf("you thought this couldn't happen in caw.c %i\n",retcmd);
        }
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
