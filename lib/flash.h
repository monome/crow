#pragma once

#include "stm32f7xx.h"

// 16kB calibration
#define CALIBRATION_LOCATION 0x0800C000
#define CALIBRATION_SECTOR   FLASH_SECTOR_3
#define CALIBRATION_SIZE     (0x4000 - 4)

// 64kB user script
#define USER_SCRIPT_LOCATION 0x08010000
#define USER_SCRIPT_SECTOR   FLASH_SECTOR_4
#define USER_SCRIPT_SIZE     (0x10000 - 4)

typedef enum { FLASH_Status_Init  = 0
             , FLASH_Status_Saved = 1
             , FLASH_Status_Dirty = 2
} FLASH_Status_t;

typedef struct FLASH_Store {
    FLASH_Status_t status;
    size_t         size;
    void*          address;
} FLASH_Store_t;

uint8_t Flash_is_user_script( void );
void Flash_clear_user_script( void );
uint8_t Flash_write_user_script( char* script, uint32_t length );
uint16_t Flash_read_user_scriptlen( void );
uint8_t Flash_read_user_script( char* buffer );

uint8_t Flash_is_calibrated( void );
void Flash_clear_calibration( void );
uint8_t Flash_write_calibration( uint8_t* data, uint32_t length );
uint8_t Flash_read_calibration( uint8_t* data, uint32_t length );
