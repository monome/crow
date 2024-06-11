#include "usb_device.h"
#include "usbd_desc.h"
#include "main.h"
#include "../Target/usbd_conf.h"

#include "ll/debug_usart.h"
#include "usbd_composite.h"

USBD_HandleTypeDef hUsbDevice;

void MX_USB_DEVICE_Init(void){
  USBD_COMPOSITE_Mount_Class();

#if (USBD_USE_HS == 1)
  if (USBD_Init(&hUsbDevice, &USBD_Desc, DEVICE_HS) != USBD_OK)
  {
    Error_Handler();
  }
#else
  if (USBD_Init(&hUsbDevice, &USBD_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
#endif
  if (USBD_RegisterClass(&hUsbDevice, &USBD_COMPOSITE) != USBD_OK)
  {
    Error_Handler();
  }
#if (USBD_USE_CDC_ACM == 1)
  if (USBD_CDC_ACM_RegisterInterface(&hUsbDevice, &USBD_CDC_ACM_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_CDC_RNDIS == 1)
  if (USBD_CDC_RNDIS_RegisterInterface(&hUsbDevice, &USBD_CDC_RNDIS_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_CDC_ECM == 1)
  if (USBD_CDC_ECM_RegisterInterface(&hUsbDevice, &USBD_CDC_ECM_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_HID_MOUSE == 1)
#endif
#if (USBD_USE_HID_KEYBOARD == 1)
#endif
#if (USBD_USE_HID_CUSTOM == 1)
  if (USBD_CUSTOM_HID_RegisterInterface(&hUsbDevice, &USBD_CustomHID_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_UAC_MIC == 1)
  if (USBD_AUDIO_MIC_RegisterInterface(&hUsbDevice, &USBD_AUDIO_MIC_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_UAC_SPKR == 1)
  if (USBD_AUDIO_SPKR_RegisterInterface(&hUsbDevice, &USBD_AUDIO_SPKR_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_UVC == 1)
  if (USBD_VIDEO_RegisterInterface(&hUsbDevice, &USBD_VIDEO_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_MSC == 1)
  if (USBD_MSC_RegisterStorage(&hUsbDevice, &USBD_Storage_Interface_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_DFU == 1)
  if (USBD_DFU_RegisterMedia(&hUsbDevice, &USBD_DFU_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_PRNTR == 1)
  if (USBD_PRNT_RegisterInterface(&hUsbDevice, &USBD_PRNT_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
#if (USBD_USE_PRNTR == 1)
  if (USBD_MIDI_RegisterInterface(&hUsbDevice, &USBD_MIDI_fops) != USBD_OK)
  {
    Error_Handler();
  }
#endif
    // printf("usbd_start\n\r");
    // U_PrintNow();
  if (USBD_Start(&hUsbDevice) != USBD_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
