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

const char* II_list_cmds( uint8_t address )
{
    return ii_list_commands(address);
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
            case II_float: *d++ = *(float*)data; data += 4; break;
            default: printf("ii_decode unmatched\n"); break;
        }
    }
    return decoded;
}

void I2C_Lead_RxCallback( uint8_t address, uint8_t cmd, uint8_t* data )
{
    printf("ii_lead_rx: cmd %i, data %i %i %i\n", cmd, data[0], data[1], data[2]);
    const II_Cmd_t* c = ii_find_command(address, cmd);
    float val;
    L_handle_ii_leadRx( address
               , cmd
               , *_II_decode_packet( &val, data, c, 0 )
               );
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
    const II_Cmd_t* c = ii_find_command(II_get_address(), cmd);
    float args[c->args];
    // run the callback directly
    L_handle_ii_followRx( cmd
                   , c->args
                   , _II_decode_packet( args, data, c, 1 )
                   );
}

uint8_t _II_make_return( uint8_t* buf, const II_Cmd_t* c, uint8_t cmd, float* data )
{
    uint8_t byte = 0;
    uint8_t* b = buf;

    uint16_t u16; int16_t s16;
    switch( c->return_type ){
        case II_u8: b[byte++] = (uint8_t)(*data++);
            break;
        case II_s8: b[byte++] = (int8_t)(*data++);
            break;
        case II_u16:
            u16 = (uint16_t)(*data++);
            b[byte++] = (uint8_t)(u16>>8);          // High byte first
            b[byte++] = (uint8_t)(u16 & 0x00FF);    // Low byte
            break;
        case II_s16:
            s16 = (int16_t)(*data++);
            u16 = *(uint16_t*)&s16;
            b[byte++] = (uint8_t)(u16>>8);          // High byte first
            b[byte++] = (uint8_t)(u16 & 0x00FF);    // Low byte
            break;
        case II_float:
            memcpy( &(b[byte]), data++, 4 );
            byte += 4;
            break;
        default: printf("no retval found\n"); return 0;
    }
    return byte;
}

void I2C_Follow_TxCallback( uint8_t* data )
{
    printf("ii_follow_tx: cmd %i, data %i, %i\n", data[0], data[1], data[2]);
    uint8_t cmd = *data++; // first data holds command
    const II_Cmd_t* c = ii_find_command(II_get_address(), cmd);
    float args[c->args];
    // run the callback directly!
    float response = L_handle_ii_followRxTx( cmd
                   , c->args
                   , _II_decode_packet( args, data, c, 1 )
                   );
    static uint8_t d[4];
    uint8_t length = _II_make_return( d, c, cmd, &response );
    //I2C_SetTxData( d, _II_type_size( c->return_type ) );
    //FIXME _II_type_size doesn't work on an auto-generated retval for a getter
    I2C_SetTxData( d, length );
}

// TODO merge with _II_make_return with a 'is_return' flag
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
                b[byte++] = (uint8_t)(u16>>8);          // High byte first
                b[byte++] = (uint8_t)(u16 & 0x00FF);    // Low byte
                break;
            case II_s16:
                s16 = (int16_t)(*data++);
                u16 = *(uint16_t*)&s16;
                b[byte++] = (uint8_t)(u16>>8);          // High byte first
                b[byte++] = (uint8_t)(u16 & 0x00FF);    // Low byte
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
