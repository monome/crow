#include "mhost.h"

#include <stm32f7xx.h>

// #include "usb_device.h"

void MHost_Init(void){
    // __HAL_RCC_GPIOB_CLK_ENABLE();

    // GPIO_InitTypeDef g;
    // g.Pin   = GPIO_PIN_12;
    // g.Mode  = GPIO_MODE_OUTPUT_PP;
    // g.Pull  = GPIO_NOPULL;
    // g.Speed = GPIO_SPEED_FAST;
    // HAL_GPIO_Init(GPIOB, &g);
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0); // immediately turn off power




    // from https://github.com/Hypnotriod/midi-box-stm32
    // MX_USB_DEVICE_Init();



}

void MHost_Task(void){
    // MIDI_ProcessUSBData(); // see /midi_router.c
}

void MHost_Power(int status){ // enable/disable +5v to connected device
  // handled by the LL usbh driver
    // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, !!status);
}

// TODO support power error flag from usb switch

/*
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

USBD_HandleTypeDef hUsbDeviceFS;

void MX_USB_DEVICE_Init(void)
{
  // Init Device Library, add supported class and start the library.
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }
}
*/
