#pragma once

#include "stm32f7xx.h"

// 16kB calibration -> using 256kB
#define CALIBRATION_LOCATION 0x08180000
#define CALIBRATION_SECTOR   FLASH_SECTOR_10 // UNUSED
#define CALIBRATION_SIZE     (0x40000 - 4)

// 64kB user script -> using 256kB
#define USER_SCRIPT_LOCATION 0x081C0000
#define USER_SCRIPT_SECTOR   FLASH_SECTOR_11
//#define USER_SCRIPT_SIZE     (0x10000 - 4)
// #define USER_SCRIPT_SIZE     (0x2000 - 4) // 8kB up to v2.1
#define USER_SCRIPT_SIZE     (0x40000 - 4) // 16kB v3.0+

typedef enum { FLASH_Status_Init  = 0
             , FLASH_Status_Saved = 1
             , FLASH_Status_Dirty = 2
} FLASH_Status_t;

typedef struct FLASH_Store {
    FLASH_Status_t status;
    size_t         size;
    void*          address;
} FLASH_Store_t;

typedef enum { USERSCRIPT_Default
             , USERSCRIPT_User
             , USERSCRIPT_Clear
} USERSCRIPT_t;

USERSCRIPT_t Flash_which_user_script( void );
void Flash_clear_user_script( void );
void Flash_default_user_script( void );
uint8_t Flash_write_user_script( char* script, uint32_t length );
uint16_t Flash_read_user_scriptlen( void );
char* Flash_read_user_scriptaddr( void );
uint8_t Flash_read_user_script( char* buffer );

uint8_t Flash_is_calibrated( void );
void Flash_clear_calibration( void );
uint8_t Flash_write_calibration( uint8_t* data, uint32_t length );
uint8_t Flash_read_calibration( uint8_t* data, uint32_t length );
