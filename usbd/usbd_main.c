#include "usbd_main.h"

#include "../ll/debug_usart.h"

USBD_HandleTypeDef USBD_Device;

void USB_CDC_Init(void)
{
    // Init Device Library
    if( USBD_Init(&USBD_Device, &VCP_Desc, 0) != USBD_OK ){
        U_PrintLn("Invalid Device handle");
    }

    // Add Supported Class
    if( USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS) != USBD_OK ){
        U_PrintLn("Invalid class handle");
    }

    // Add CDC Interface Class
    if( USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops) != USBD_OK ){
        U_PrintLn("Failed to register interface");
    }

    // Start Device Process
    USBD_Start(&USBD_Device);
}

extern PCD_HandleTypeDef hpcd;

#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
    HAL_PCD_IRQHandler(&hpcd);
}

extern TIM_HandleTypeDef USBTimHandle;

void TIMu_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&USBTimHandle);
}
