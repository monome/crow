#pragma once

#include <stm32f7xx.h>

#include "../ll/i2c.h"

#define WS_NULL   0
#define WS_REC    1
#define WS_PLAY   2
#define WS_LOOP   3
#define WS_CUE    4
#define WS_UI     5

#define ii_GET             128  // cmd >= are getter requests

typedef enum{ II_CROW  = 0x01
            , II_CROW2 = 0x02
            , II_CROW3 = 0x03
            , II_CROW4 = 0x04
} ii_ADDR_t;

uint8_t ii_init( uint8_t address );
void ii_deinit( void );

const char* ii_list_modules( void );
const char* ii_list_cmds( uint8_t address );

void ii_set_pullups( uint8_t state );

uint8_t ii_get_address( void );
void ii_set_address( uint8_t address );

// callbacks from i2c LL library
void I2C_Lead_RxCallback( uint8_t address, uint8_t cmd, uint8_t* data );
void I2C_Follow_RxCallback( uint8_t* data );
void I2C_Follow_TxCallback( uint8_t* data );
// ^^ good




uint8_t* ii_processFollowRx( void );
uint8_t* ii_processLeadRx( void );




// good
uint8_t ii_broadcast( uint8_t address
                    , uint8_t cmd
                    , float*  data
                    );
uint8_t ii_query( uint8_t address
                , uint8_t cmd
                , float*  data
                );
