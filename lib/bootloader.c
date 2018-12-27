#include "bootloader.h"

#include <stm32f7xx.h>

#define GOTO_BOOT_ADDRESS  0x20006666
#define GOTO_BOOT_MAGICNUM 0x66666666

void bootloader_enter( void )
{
    // TODO de-init everything
    *(uint32_t*)(GOTO_BOOT_ADDRESS) = GOTO_BOOT_MAGICNUM;
    NVIC_SystemReset();
    while(1){}
}

void bootloader_restart( void )
{
    NVIC_SystemReset();
    while(1){}
}
