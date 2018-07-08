#include "caw.h"

#include <string.h>

#include "../usbd/usbd_cdc_interface.h"
#include "../ll/debug_usart.h"

/*uint8_t Caw_Init( fnptr of callback )
{
    USBD_Init();
    // set static void(*) which Caw_receive_callback() forward all info to
    // alt, we could just send a *buffer that we copy data into & check in scheduler
}*/

// rawtext & luachunk expect a terminating \0 char
void Caw_send_rawtext( char* text )
{
    uint32_t len = strlen(text);
    uint8_t s[len + 3];
    s[0]     = ';';
    memcpy( &s[1], text, len );
    s[len+1] = '\n';
    s[len+2] = '\r';

    USB_tx_enqueue( s
                  , len + 3
                  );
}

void Caw_send_luachunk( char* text )
{
    uint32_t len = strlen(text);
    uint8_t s[len + 3];
    s[0]     = '#';
    memcpy( &s[1], text, len );
    s[len+1] = '\n';
    s[len+2] = '\r';

    USB_tx_enqueue( s
                  , len + 3
                  );
}

void Caw_send_value( uint8_t type, float value )
{
    //
}

uint8_t Caw_try_receive( void )
{
    uint8_t isdata = 0;
    static uint8_t* buf;
    static uint32_t len;

    if( USB_rx_dequeue( &buf, &len ) ){
        isdata = 1;
        // switch on buf[0] to select action
        Caw_receive_rawtext_callback( buf, len );
    }
    return isdata;
}

__weak void Caw_receive_rawtext_callback( uint8_t* buf, uint32_t len )
{
    char s[len+1];
    memcpy( s, buf, len );
    s[len] = '\0'; // ensure termination

    U_PrintLn(s);
}
