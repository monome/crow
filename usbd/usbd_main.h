#pragma once

#include "stm32f7xx_hal.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"

#define USE_USB_FS

extern USBD_HandleTypeDef USBD_Device;

void USB_CDC_Init(int timer_index);
void USB_CDC_DeInit(void);

void OTG_FS_IRQHandler(void);
