#include "ii.h"

#include "../ll/debug_usart.h"
#include <stm32f7xx_hal.h>
#include <string.h>
#include "ii_modules.h"

// WithType implementation (move to separate file)
#define II_MAX_BROADCAST_LEN 4 // just u8 or u16

// public defns
uint8_t II_init( uint8_t address )
{
    if( address != II_FOLLOW
     && address != II_LEADER1
     && address != II_LEADER2
     && address != II_LEADER3
      ){ address = II_FOLLOW; } // ensure a valid address
    if( I2C_Init( (uint8_t)address ) ){
        U_PrintLn("I2C Failed to Init");
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










II_ADDR_t II_get_mode( void )
{
    return (II_ADDR_t)I2C_GetAddress();
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

// Transfer functions
uint8_t testo = 7;
// Handles both follower cases
void I2C_RxCpltCallback( uint8_t* data )
{
    if( data[0] >= II_GET ){
        // bounds check cmd
        // same as II_process() in terms of fnptr
        // but fns just grab a data pointer
        I2C_SetTxData( &testo, 1 );
    } else {
        I2C_BufferRx( data );
    }
}
// Leader Transmit
void II_broadcast( II_ADDR_t address
                 , uint8_t   cmd
                 , uint8_t*  data
                 , uint8_t   size
                 )
{
    //U_PrintLn("broadcast");
    // need a queue here to allow repetitive calls to the fn
    // best to implement the DMA
    static uint8_t tx_buf[1+II_MAX_BROADCAST_LEN];
    tx_buf[0] = cmd;
    memcpy( &(tx_buf[1])
          , data
          , size
          );
    if( I2C_LeadTx( address
                  , tx_buf
                  , size+1
                  ) ){ U_PrintLn("Leader Tx Failed"); }
}

// Leader Request
// consider how this should be integrated. what args??
void II_query( void )
{
    // can remove static after adding queue to LL driver
    const uint8_t size = 1+II_MAX_BROADCAST_LEN;
    static uint8_t rx_buf[1+II_MAX_BROADCAST_LEN];
    rx_buf[0] = 1; // choose knob 1
    for( uint8_t i=1; i<size; i++ ){
        rx_buf[i] = 0;
    }
    if( I2C_LeadRx( TI_1 // TelexI // send this as an address
                  , rx_buf
                  , 2 // size limit
                  ) ){ U_PrintLn("Leader Rx Failed"); }
}
