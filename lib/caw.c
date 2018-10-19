#include "caw.h"

#include <string.h>

#include "../usbd/usbd_cdc_interface.h"
#include "../ll/debug_usart.h"
#include "../../wrLib/str_buffer.h"

str_buffer_t str_buf;

uint8_t Caw_Init( void ) // should take a fnptr to L_cb_repl
{
    //USBD_Init();
    // set static void(*) which Caw_receive_callback() forward all info to
    // alt, we could just send a *buffer that we copy data into & check in scheduler
    //
    str_buffer_init( &str_buf, 1023 );
    return 0;
}

void Caw_DeInit( void )
{
    str_buffer_deinit( &str_buf );
}

void Caw_send_rawtext( char* text, uint32_t len )
{
    USB_tx_enqueue( (unsigned char*)text, len );
}
// luachunk expects a terminating \0 char
void Caw_send_luachunk( char* text )
{
    //uint32_t len = strlen(text);
    //uint8_t s[len + 3];
    //s[0]     = '#';
    //memcpy( &s[1], text, len );
    //s[len+1] = '\n';
    //s[len+2] = '\r';

    //USB_tx_enqueue( s
    //              , len + 3
    //              );
}

void Caw_send_value( uint8_t type, float value )
{
    //
}

void Caw_try_receive( void )
{
    static uint8_t* buf;
    static uint32_t len;

    if( USB_rx_dequeue( &buf, &len ) ){
        str_buffer_enqueue( &str_buf,  )
        //TODO need to echo typed messages to tty?
        //TODO push into a str_buf (queue) with a big size >1024
        //     then only execute the callback if the last char is \n
        Caw_receive_callback( buf, len );
    }
}

//__weak void Caw_receive_callback( uint8_t* buf, uint32_t len )
void Caw_receive_callback( uint8_t* buf, uint32_t len )
{
    char s[len+1];
    memcpy( s, buf, len );
    s[len] = '\0'; // ensure termination

    if( len == 1 ){
        U_Print("char: "); U_PrintU8(*buf);
    } else {
        U_Print("not a single char but: ");
        for( uint8_t i=0; i<len; i++ ){
            U_PrintU8n(buf[i]);
            U_Print(",");
        }
        U_PrintLn("");
    }
    Caw_send_rawtext( (char*)buf, len );
}
