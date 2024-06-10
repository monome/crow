#pragma once

#include "stm32f7xx_hal.h" // CPU_count;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

void MX_USB_OTG_FS_PCD_Init(void);
