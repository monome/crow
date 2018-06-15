#include "caw.h"

#include <string.h>

#include "../usbd/usbd_cdc_interface.h"

/*uint8_t Caw_Init( fnptr of callback )
{
    USBD_Init();
    // set static void(*) which Caw_receive_callback() forward all info to
    // alt, we could just send a *buffer that we copy data into & check in scheduler
}*/

// rawtext & luachunk expect a terminating \0 char
void Caw_send_rawtext( char* text )
{
    uint32_t len = strlen(text) + 1; // +1 is event type
    uint8_t s[len];
    s[0] = ';';
    memcpy( &s[1], text, len );

    USB_tx_enqueue( s
                  , len
                  );
}

void Caw_send_luachunk( char* text )
{
    uint32_t len = strlen(text) + 1; // +1 is event type
    uint8_t s[len];
    s[0] = '#';
    memcpy( &s[1], text, len );

    USB_tx_enqueue( s
                  , len
                  );

}

void Caw_send_value( uint8_t type, float value )
{
    //
}

void Caw_receive_callback( uint8_t* buf, uint32_t len )
{
    // Any received message triggers this callback
}
