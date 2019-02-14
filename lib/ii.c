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
    if( address != II_CROW
     && address != II_CROW2
     && address != II_CROW3
     && address != II_CROW4 ){ address = II_CROW; } // ensure a valid address
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
void II_set_pullups( uint8_t state )
{
    I2C_SetPullups(state);
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

float* _II_decode_packet( float* decoded
                        , uint8_t* data
                        , const II_Cmd_t* c
                        , int is_following
                        )
{
    float* d = decoded;
    uint16_t u16 = 0;
    for( int i=0; i<(is_following ? c->args : 1); i++ ){
        switch( (is_following) ? c->argtype[i] : c->return_type ){
            case II_u8: *d++ = (float)(*data++); break;
            case II_s8: *d++ = (float)(*(int8_t*)data++); break;
            case II_u16:
                u16  = ((uint16_t)*data++)<<8;
                u16 |= *data++;
                *d++ = (float)u16;
                break;
            case II_s16:
                u16  = ((uint16_t)*data++)<<8;
                u16 |= *data++;
                *d++ = (float)*(int16_t*)&u16;
                break;
            case II_float: *d++ = *data; data += 4; break;
            default: printf("ii_decode unmatched\n"); break;
        }
    }
    return decoded;
}

uint8_t rx_address;

// Handles both follower cases
void I2C_Lead_RxCallback( uint8_t address, uint8_t cmd, uint8_t* data )
{
    printf("ii_lead_rx: addr %i, cmd %i, data %i, %i, %i\n", address, cmd, data[0], data[1], data[2]);
    const II_Cmd_t* c = ii_find_command(address, cmd);

    float val;
    L_handle_ii( address
               , cmd
               , *_II_decode_packet( &val, data, c, 0 )
               );
    // TODO note: the 'follow' case above allows multiple vals!
    // TODO should set a flag, and callback from main loop
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

void I2C_Follow_RxCallback( uint8_t* data )
{
    printf("ii_follow_rx: cmd %i, data %i, %i\n", data[0], data[1], data[2]);
    uint8_t cmd = *data++; // first data holds command
    const II_Cmd_t* c = ii_find_command(0x28, cmd);
    float args[c->args];
    L_handle_iiself( cmd
                   , c->args
                   , _II_decode_packet( args, data, c, 1 )
                   );
    // nb: we run the callback directly bc all devices on the i2c bus can
    // potentially lockup while the transfer is in progress.
    static uint8_t d[2] = {0,0}; // FIXME this needs to be response from lua
    //I2C_SetTxData( d, _II_type_size( c->return_type ) );
    //FIXME _II_type_size doesn't work on an auto-generated retval for a getter
    I2C_SetTxData( d, 1 );
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
