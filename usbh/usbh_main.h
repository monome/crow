#pragma once

#include "usbh_cdc.h" // USBH_CDC_CLASS

void USBHost_Init(void);
// return length of data available
int USBHost_BG_Task(void);

void USBHost_Send(uint8_t* data, size_t len);
size_t USBHost_Get_Received(uint8_t** data);
