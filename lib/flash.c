#include "flash.h"

#include "../ll/debug_usart.h"

#define USER_MAGIC 0xA  // bit pattern

// private declarations
static void clear_flash( uint32_t sector, uint32_t location );
static uint32_t version12b( void );

// USER LUA SCRIPT //

uint8_t Flash_is_user_script( void )
{
    return (USER_MAGIC == (0xF & (*(__IO uint32_t*)USER_SCRIPT_LOCATION)));
}

void Flash_clear_user_script( void )
{
    printf("clear user script\n");
    clear_flash( USER_SCRIPT_SECTOR, USER_SCRIPT_LOCATION );
}

uint8_t Flash_write_user_script( char* script, uint32_t length )
{
    if( length > USER_SCRIPT_SIZE ){ return 1; } // ERROR: Script too long

// clear the flash
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef erase_setup =
		{ .TypeErase    = FLASH_TYPEERASE_SECTORS
		, .Sector       = USER_SCRIPT_SECTOR
		, .NbSectors    = 1
		, .VoltageRange = FLASH_VOLTAGE_RANGE_3
		};
    uint32_t error_status;
	HAL_FLASHEx_Erase( &erase_setup, &error_status );

// set status word
    uint32_t sd_addr = USER_SCRIPT_LOCATION;
	HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD
					 , sd_addr
					 , USER_MAGIC          // user script present
                     | (version12b() << 4) // version control
                     | (length << 16)      // length in bytes
					 );
// program script
    length >>= 2; length++;
    while( length ){
        sd_addr += 4;
	    HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD
	    				 , sd_addr
	    				 , *(uint32_t*)script
	    				 );
        script += 4;
        length--;
    }
	HAL_FLASH_Lock();
    return 0;
}

uint16_t Flash_read_user_scriptlen( void )
{
    return (*((__IO uint16_t*)USER_SCRIPT_LOCATION + 1));
}

uint8_t Flash_read_user_script( char* buffer )
{
    if( !Flash_is_user_script() ){ return 1; } // no script
    // FIXME: need to add 1 to length for null char?

    uint16_t word_length = ((*(__IO uint32_t*)USER_SCRIPT_LOCATION) >> 16) >> 2;
    word_length++;

    uint32_t sd_addr = USER_SCRIPT_LOCATION + 4;
    uint32_t* word_buffer = (uint32_t*)buffer;
    while( word_length-- ){
        *word_buffer++ = (*(__IO uint32_t*)sd_addr);
        sd_addr += 4;
    }
    return 0;
}

// CALIBRATION //

uint8_t Flash_is_calibrated( void )
{
    return ((*(__IO uint32_t*)CALIBRATION_LOCATION) == USER_MAGIC );
}

void Flash_clear_calibration( void )
{
    clear_flash( CALIBRATION_SECTOR, CALIBRATION_LOCATION );
}

uint8_t Flash_write_calibration( uint8_t* data, uint32_t length )
{
    if( length > CALIBRATION_SIZE ){ return 1; } // ERROR: data too long

    // TODO: save the module version here? can also be queried from lua

// clear the flash
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef erase_setup =
		{ .TypeErase    = FLASH_TYPEERASE_SECTORS
		, .Sector       = CALIBRATION_SECTOR
		, .NbSectors    = 1
		, .VoltageRange = FLASH_VOLTAGE_RANGE_3
		};
    uint32_t error_status;
	HAL_FLASHEx_Erase( &erase_setup, &error_status );

// set status word
    uint32_t sd_addr = CALIBRATION_LOCATION;
	HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD
					 , sd_addr
					 , USER_MAGIC
					 );
// program script
    length >>= 2; length++;
    while( length ){
        sd_addr += 4;
	    HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD
	    				 , sd_addr
	    				 , *(uint32_t*)data
	    				 );
        data += 4;
        length--;
    }
	HAL_FLASH_Lock();
    return 0;
}

uint8_t Flash_read_calibration( uint8_t* data, uint32_t length )
{
    if( !Flash_is_calibrated() ){ return 1; } // no calibration

    uint16_t word_length = length >> 2;
    word_length++;

    uint32_t sd_addr = CALIBRATION_LOCATION + 4; // skip status word
    uint32_t* word_buffer = (uint32_t*)data;
    while( word_length-- ){
        *word_buffer++ = (*(__IO uint32_t*)sd_addr);
        sd_addr += 4;
    }
    return 0;
}


// private fns

static void clear_flash( uint32_t sector, uint32_t location )
{
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef erase_setup =
		{ .TypeErase    = FLASH_TYPEERASE_SECTORS
		, .Sector       = sector
		, .NbSectors    = 1
		, .VoltageRange = FLASH_VOLTAGE_RANGE_3
		};
    uint32_t error_status;
	HAL_FLASHEx_Erase( &erase_setup, &error_status );
	HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD
					 , location
					 , 0 // no script | zero length
					 );
    // TODO error check
	HAL_FLASH_Lock();
}

static uint32_t version12b( void )
{
    const uint32_t c = (uint32_t)( (VERSION[1]-0x30)<<8
                                 | (VERSION[3]-0x30)<<4
                                 | (VERSION[5]-0x30)
                                 ) & 0xFFF; // mask for safety
    return c;
}
