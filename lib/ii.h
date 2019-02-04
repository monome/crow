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

uint8_t II_get_address( void );
void II_set_address( uint8_t address );
// ^^ good



// unknown >>
void I2C_RxCpltCallback( uint8_t address, uint8_t cmd, uint8_t* data );

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
