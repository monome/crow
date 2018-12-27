#pragma once

#include "stm32f7xx.h"

// 16kB location
#define USER_SCRIPT_LOCATION 0x0800C000
#define USER_SCRIPT_SECTOR   FLASH_SECTOR_3
#define USER_SCRIPT_SIZE     (0x4000 - 4)

// 64kB location
//#define USER_SCRIPT_LOCATION 0x08010000
//#define USER_SCRIPT_SECTOR   FLASH_SECTOR_4
//#define USER_SCRIPT_SIZE     (0xFFFF - 4)

uint8_t Flash_is_user_script( void );
void Flash_clear_user_script( void );
uint8_t Flash_write_user_script( char* script, uint32_t length );
uint8_t Flash_read_user_script( char* buffer, uint16_t* len );
