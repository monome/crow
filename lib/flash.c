#include "flash.h"

#include "../ll/debug_usart.h"

uint8_t Flash_is_user_script( void )
{
    return (0x1 & (*(__IO uint32_t*)USER_SCRIPT_LOCATION));
}

void Flash_clear_user_script( void )
{
    U_PrintLn("clear user script");
	HAL_FLASH_Unlock();
	FLASH_EraseInitTypeDef erase_setup =
		{ .TypeErase    = FLASH_TYPEERASE_SECTORS
		, .Sector       = USER_SCRIPT_SECTOR
		, .NbSectors    = 1
		, .VoltageRange = FLASH_VOLTAGE_RANGE_3
		};
    uint32_t error_status;
	HAL_FLASHEx_Erase( &erase_setup, &error_status );
	HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD
					 , USER_SCRIPT_LOCATION
					 , 0 // no script | zero length
					 );
    // TODO error check
	HAL_FLASH_Lock();
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
					 , 1 | (length << 16) // script present | length in bytes
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

uint8_t Flash_read_user_script( char* buffer, uint16_t* len )
{
    if( !Flash_is_user_script() ){ return 1; } // no script
    // FIXME: need to add 1 to length for null char?
    *len = ((*(__IO uint32_t*)USER_SCRIPT_LOCATION) >> 16);

    uint16_t word_length = *len >> 2;
    word_length++;

    uint32_t sd_addr = USER_SCRIPT_LOCATION + 4;
    uint32_t* word_buffer = (uint32_t*)buffer;
    while( word_length-- ){
        *word_buffer++ = (*(__IO uint32_t*)sd_addr);
        sd_addr += 4;
    }
    return 0;
}
