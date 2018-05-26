#pragma once

#include "stm32f7xx_hal.h"
#include <stm32f7xx.h>

typedef enum { FLASH_Status_Init  = 0
             , FLASH_Status_Saved = 1
             , FLASH_Status_Dirty = 2
} FLASH_Status_t;

typedef struct FLASH_Store {
    FLASH_Status_t status;
    size_t         size;
    void*          address;
} FLASH_Store_t;
