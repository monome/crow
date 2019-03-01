#pragma once

#include <stm32f7xx.h>

#include "../ll/i2c.h"

#define WS_NULL   0
#define WS_REC    1
#define WS_PLAY   2
#define WS_LOOP   3
#define WS_CUE    4
#define WS_UI     5

#define II_GET             128  // cmd >= are getter requests

typedef enum{ II_CROW  = 0x78
            , II_CROW2 = 0x79
            , II_CROW3 = 0x7a
            , II_CROW4 = 0x7b
} II_ADDR_t;

uint8_t II_init( uint8_t address );
void II_deinit( void );

const char* II_list_modules( void );
const char* II_list_cmds( uint8_t address );

void II_set_pullups( uint8_t state );

uint8_t II_get_address( void );
void II_set_address( uint8_t address );

// callbacks from i2c LL library
void I2C_Lead_RxCallback( uint8_t address, uint8_t cmd, uint8_t* data );
void I2C_Follow_RxCallback( uint8_t* data );
void I2C_Follow_TxCallback( uint8_t* data );
// ^^ good




uint8_t* II_processFollowRx( void );
uint8_t* II_processLeadRx( void );




// good
uint8_t II_broadcast( uint8_t address
                    , uint8_t cmd
                    , float*  data
                    );
uint8_t II_query( uint8_t address
                , uint8_t cmd
                , float*  data
                );
