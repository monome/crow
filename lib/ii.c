#include "ii.h"

#include <stm32f7xx_hal.h>
#include <string.h>
#include "../build/ii_c_layer.h"

#include "lualink.h"

// WithType implementation (move to separate file)
#define II_MAX_BROADCAST_LEN 4 // just u8 or u16

// public defns
uint8_t ii_init( uint8_t address )
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
void ii_deinit( void )
{
    I2C_DeInit();
}

const char* ii_list_modules( void )
{
    return ii_module_list;
}

const char* ii_list_cmds( uint8_t address )
{
    return ii_list_commands(address);
}

void ii_set_pullups( uint8_t state )
{
    I2C_SetPullups(state);
}

uint8_t ii_get_address( void )
{
    return I2C_GetAddress();
}

void ii_set_address( uint8_t address )
{
    I2C_SetAddress( address );
}

uint8_t* ii_processFollowRx( void )
{
    uint8_t* pRetval = NULL;
    if( I2C_FollowBufferNotEmpty() ){
        pRetval = I2C_PopFollowBuffer();
    }
    return pRetval; // NULL for finished
}
uint8_t* ii_processLeadRx( void )
{
    uint8_t* pRetval = NULL;
    if( I2C_LeadBufferNotEmpty() ){
        pRetval = I2C_PopLeadBuffer();
    }
    return pRetval; // NULL for finished
}

/////////////////////////////////////
// ii Type Encode/Decode

static uint8_t type_size( ii_Type_t t )
{
    switch(t){ case ii_void:  return 0;
               case ii_u8:    return 1;
               case ii_s8:    return 1;
               case ii_u16:   return 2;
               case ii_s16:   return 2;
               case ii_s16V:  return 2;
               case ii_float: return 4;
    }
    return 0;
}

static float decode( uint8_t* data, ii_Type_t type )
{
    float val = 0; // return value default to zero
    uint16_t u16 = 0;
    switch( type ){
        case ii_u8:
            val = (float)(*data++);
            break;
        case ii_s8:
            val = (float)(*(int8_t*)data++);
            break;
        case ii_u16:
            u16  = ((uint16_t)*data++)<<8;
            u16 |= *data++;
            val = (float)u16;
            break;
        case ii_s16:
            u16  = ((uint16_t)*data++)<<8;
            u16 |= *data++;
            val = (float)*(int16_t*)&u16;
            break;
        case ii_s16V:
            u16  = ((uint16_t)*data++)<<8;
            u16 |= *data++;
            val = ((float)*(int16_t*)&u16)/1638.4; // Scale Teletype down to float
            break;
        case ii_float:
            val = *(float*)data;
            break;
        default: printf("ii_decode unmatched\n"); break;
    }
    return val;
}

static uint8_t pack_data( uint8_t* dest, ii_Type_t type, float data )
{
    uint8_t len = 0;
    uint8_t* d = dest;

    uint16_t u16; int16_t s16;
    switch( type ){
        case ii_u8: d[len++] = (uint8_t)data;
            break;
        case ii_s8: d[len++] = (int8_t)data;
            break;
        case ii_u16:
            u16 = (uint16_t)data;
            d[len++] = (uint8_t)(u16>>8);          // High byte first
            d[len++] = (uint8_t)(u16 & 0x00FF);    // Low byte
            break;
        case ii_s16:
            s16 = (int16_t)data;
            u16 = *(uint16_t*)&s16;
            d[len++] = (uint8_t)(u16>>8);          // High byte first
            d[len++] = (uint8_t)(u16 & 0x00FF);    // Low byte
        case ii_s16V:
            s16 = (int16_t)(data * 1638.4);        // Scale float up to Teletype
            u16 = *(uint16_t*)&s16;
            d[len++] = (uint8_t)(u16>>8);          // High byte first
            d[len++] = (uint8_t)(u16 & 0x00FF);    // Low byte
            break;
        case ii_float:
            memcpy( &(d[len]), &data, 4 );
            len += 4;
            break;
        default: printf("no retval found\n"); return 0;
            // FIXME: should this really print directly? or pass to caller?
    }
    return len;
}

float* _ii_decode_packet( float* decoded
                        , uint8_t* data
                        , const ii_Cmd_t* c
                        , int is_following
                        )
{
    float* d = decoded;
    if( is_following ){
        int len = 0;
        for( int i=0; i<(c->args); i++ ){
            *d++ = decode( &data[len], c->argtype[i] );
            len += type_size( c->argtype[i] );
        }
    } else {
        *d = decode( data, c->return_type );
    }
    return decoded;
}

void I2C_Lead_RxCallback( uint8_t address, uint8_t cmd, uint8_t* data )
{
    printf("ii_lead_rx: cmd %i, data %i %i %i\n", cmd, data[0], data[1], data[2]);
    ii_unpickle(&address, &cmd, data);
    const ii_Cmd_t* c = ii_find_command(address, cmd);
    float val;
    L_handle_ii_leadRx( address
               , cmd
               , *_ii_decode_packet( &val, data, c, 0 )
               );
}

void I2C_Follow_RxCallback( uint8_t* data )
{
    printf("ii_follow_rx: cmd %i, data %i, %i\n", data[0], data[1], data[2]);
    uint8_t cmd = *data++; // first data holds command
    const ii_Cmd_t* c = ii_find_command(ii_get_address(), cmd);
    float args[c->args];
    // run the callback directly
    // TODO: THIS GOES IN THE EVENT SYSTEM
    L_handle_ii_followRx( cmd
                   , c->args
                   , _ii_decode_packet( args, data, c, 1 )
                   );
}

void I2C_Follow_TxCallback( uint8_t* data )
{
    printf("ii_follow_tx: cmd %i, data %i, %i\n", data[0], data[1], data[2]);
    uint8_t cmd = *data++; // first data holds command
    const ii_Cmd_t* c = ii_find_command(ii_get_address(), cmd);
    float args[c->args];
    // run the callback directly!
    float response = L_handle_ii_followRxTx( cmd
                   , c->args
                   , _ii_decode_packet( args, data, c, 1 )
                   );
    static uint8_t d[4]; // TODO: only allow a single retval currently
    uint8_t length = pack_data( d, c->return_type, response );
    //I2C_SetTxData( d, type_size( c->return_type ) );
    //FIXME type_size doesn't work on an auto-generated retval for a getter
    I2C_SetTxData( d, length );
}

uint8_t _ii_make_packet( uint8_t* dest, const ii_Cmd_t* c, uint8_t cmd, float* data )
{
    uint8_t len = 0;
    dest[len++] = cmd; // first byte is command
    for( int i=0; i<(c->args); i++ ){
        len += pack_data( &dest[len], c->argtype[i], *data++ );
    }
    return len;
}

// Leader Transmit
uint8_t ii_broadcast( uint8_t address
                    , uint8_t cmd
                    , float*  data
                    )
{
    // need a queue here to allow repetitive calls to the fn
    // best to implement the DMA
    static uint8_t tx_buf[1+II_MAX_BROADCAST_LEN]; // empty buf to write
    const ii_Cmd_t* c = ii_find_command(address, cmd);
    uint8_t len = _ii_make_packet( tx_buf
                    , c
                    , cmd
                    , data
                    );
    if( len == 0 ){ return 2; }
    ii_pickle( &address, tx_buf, &len ); // mutate in place
    if( I2C_LeadTx( address
                  , tx_buf
                  , len
                  ) ){ return 1; }
    return 0;
}

// Leader Request
// consider how this should be integrated. what args??
uint8_t ii_query( uint8_t address
                , uint8_t cmd
                , float*  data
                )
{
    static uint8_t rx_buf[1+II_MAX_BROADCAST_LEN];
    const ii_Cmd_t* c = ii_find_command(address, cmd);
    uint8_t byte = _ii_make_packet( rx_buf
                    , c
                    , cmd
                    , data
                    );
    if( byte == 0 ){ return 2; }
    ii_pickle( &address, rx_buf, &byte ); // mutate in place
    if( I2C_LeadRx( address
                  , rx_buf
                  , byte
                  , type_size( c->return_type )
                  ) ){ return 1; }
    return 0;
}
