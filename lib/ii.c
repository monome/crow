#include "ii.h"

#include <stm32f7xx_hal.h>
#include <string.h>
#include "../build/ii_modules.h"

#include "lualink.h"

// WithType implementation (move to separate file)
#define II_MAX_BROADCAST_LEN 4 // just u8 or u16

// public defns
uint8_t II_init( uint8_t address )
{
    if( address != II_FOLLOW
     && address != II_LEADER1
     && address != II_LEADER2
     && address != II_LEADER3 ){ address = II_FOLLOW; } // ensure a valid address
    if( I2C_Init( (uint8_t)address ) ){
        printf("I2C Failed to Init\n");
    }
    return address;
}
void II_deinit( void )
{
    I2C_DeInit();
}

const char* II_list_modules( void )
{
    return ii_module_list;
}

uint8_t II_get_address( void )
{
    return I2C_GetAddress();
}

void II_set_address( uint8_t address )
{
    I2C_SetAddress( address );
}







uint8_t* II_processFollowRx( void )
{
    uint8_t* pRetval = NULL;
    if( I2C_FollowBufferNotEmpty() ){
        pRetval = I2C_PopFollowBuffer();
    }
    return pRetval; // NULL for finished
}
uint8_t* II_processLeadRx( void )
{
    uint8_t* pRetval = NULL;
    if( I2C_LeadBufferNotEmpty() ){
        pRetval = I2C_PopLeadBuffer();
    }
    return pRetval; // NULL for finished
}




float _II_decode_packet( uint8_t* data, const II_Cmd_t* c )
{
    //uint8_t* b = buf;
    //b[byte++] = cmd;

    float retval = 0.0;
    uint16_t u16 = 0;
    switch( c->return_type ){
        case II_u8: retval = (float)(*data); break;
        case II_s8: retval = (float)(*(int8_t*)data); break;
        case II_u16:
            u16  = ((uint16_t)*data++)<<8;
            u16 |= *data;
            retval = (float)u16;
            break;
        case II_s16:
            u16  = ((uint16_t)*data++)<<8;
            u16 |= *data;
            retval = (float)*(int16_t*)&u16;
            break;
        case II_float: retval = *data; break;
        default: retval = 0.0; printf("dfaul\n"); break;
    }
    return retval;
}

uint8_t rx_address;

// Transfer functions
// Handles both follower cases
void I2C_RxCpltCallback( uint8_t address, uint8_t cmd, uint8_t* data )
{
    const II_Cmd_t* c = ii_find_command(address, cmd);
    float val = _II_decode_packet( data, c );

    // FIXME should set a flag, and callback from main loop
    L_handle_ii( address, cmd, val );








    //if( data[0] >= II_GET ){
    //    // bounds check cmd
    //    // same as II_process() in terms of fnptr
    //    // but fns just grab a data pointer
    //    I2C_SetTxData( &testo, 1 );
    //} else {
    //    I2C_BufferRx( data );
    //}
}







uint8_t _II_type_size( II_Type_t t )
{
    switch(t){ case II_void:  return 0;
               case II_u8:    return 1;
               case II_s8:    return 1;
               case II_u16:   return 2;
               case II_s16:   return 2;
               case II_float: return 4;
    }
    return 0;
}

uint8_t _II_make_packet( uint8_t* buf, const II_Cmd_t* c, uint8_t cmd, float* data )
{
    uint8_t byte = 0;
    uint8_t* b = buf;
    b[byte++] = cmd;

    for( int i=0; i<(c->args); i++ ){
        uint16_t u16; int16_t s16;
        switch( c->argtype[i] ){
            case II_u8: b[byte++] = (uint8_t)(*data++);
                break;
            case II_s8: b[byte++] = (int8_t)(*data++);
                break;
            case II_u16:
                u16 = (uint16_t)(*data++);
                memcpy( &(b[byte]), &u16, 2 );
                byte += 2;
                break;
            case II_s16:
                s16 = (int16_t)(*data++);
                memcpy( &(b[byte]), &s16, 2 );
                byte += 2;
                break;
            case II_float:
                memcpy( &(b[byte]), data++, 4 );
                byte += 4;
                break;
            default: return 0;
        }
    }
    return byte;
}

// Leader Transmit
uint8_t II_broadcast( uint8_t address
                    , uint8_t cmd
                    , float*  data
                    )
{
    // need a queue here to allow repetitive calls to the fn
    // best to implement the DMA
    static uint8_t tx_buf[1+II_MAX_BROADCAST_LEN];
    const II_Cmd_t* c = ii_find_command(address, cmd);
    uint8_t byte = _II_make_packet( tx_buf
                   , c
                   , cmd
                   , data
                   );
    if( byte == 0 ){ return 2; }
    if( I2C_LeadTx( address
                  , tx_buf
                  , byte
                  ) ){ return 1; }
    return 0;
}

// Leader Request
// consider how this should be integrated. what args??
uint8_t II_query( uint8_t address
                , uint8_t cmd
                , float*  data
                )
{
    static uint8_t rx_buf[1+II_MAX_BROADCAST_LEN];
    const II_Cmd_t* c = ii_find_command(address, cmd);
    uint8_t byte = _II_make_packet( rx_buf
                   , c
                   , cmd
                   , data
                   );
    if( byte == 0 ){ return 2; }
    if( I2C_LeadRx( address
                  , rx_buf
                  , byte
                  , _II_type_size( c->return_type )
                  ) ){ return 1; }
    return 0;
}
