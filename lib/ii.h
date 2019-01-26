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

typedef enum
    { II_FOLLOW  = 0xE2 // 0xE2 was before which is really 0x62 is this 0x71<<1 ?
    , II_LEADER1 = 0x72 // broadcast physical layer
    , II_LEADER2 = 0x73
    , II_LEADER3 = 0x74
    // Telex Expanders
    , TO_1       = (0x60 << 1)
    , TI_1       = (0x68 << 1)
    } II_ADDR_t;

uint8_t II_init( uint8_t address );
void II_deinit( void );

const char* II_list_modules( void );






II_ADDR_t II_get_mode( void );

void I2C_RxCpltCallback( uint8_t* data );

uint8_t* II_processFollowRx( void );
uint8_t* II_processLeadRx( void );
void II_broadcast( II_ADDR_t address
                 , uint8_t cmd
                 , uint8_t* data
                 , uint8_t size
                 );
void II_query( void );
