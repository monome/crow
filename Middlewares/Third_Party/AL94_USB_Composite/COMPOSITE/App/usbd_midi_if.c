#include "usbd_midi_if.h"

#define MIDI_RX_DATA_SIZE MIDI_EPOUT_SIZE
static uint8_t usb_rx_buffer[MIDI_RX_DATA_SIZE] = {0};

extern USBD_HandleTypeDef hUsbDevice;

static int8_t MIDI_Init(void);
static int8_t MIDI_DeInit(void);
static int8_t MIDI_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t MIDI_Receive(uint8_t *pbuf, uint32_t *Len);
static int8_t MIDI_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum);

USBD_MIDI_ItfTypeDef USBD_MIDI_fops = {MIDI_Init,
                                       MIDI_DeInit,
                                       MIDI_Control,
                                       MIDI_Receive,
                                       MIDI_TransmitCplt};

#include "lib/caw.h"
static int8_t MIDI_Init(void){
  Caw_printf("midi init\n\r");
  USBD_MIDI_SetRxBuffer(&hUsbDevice, usb_rx_buffer);
  return (USBD_OK);
}

static int8_t MIDI_DeInit(void){
  return (USBD_OK);
}
static int8_t MIDI_Control(uint8_t cmd, uint8_t *pbuf, uint16_t length){
  Caw_printf("midi control\n\r");
  return (USBD_OK);
}
static int8_t MIDI_Receive(uint8_t *pbuf, uint32_t *Len){
  Caw_printf("midi receive\n\r");

  // uint32_t len = *Len;
  // handle len worth of bytes in pbuf
  // just print to Caw for now

  memset(usb_rx_buffer, 0, MIDI_RX_DATA_SIZE);
  
  return (USBD_OK);
}
static int8_t MIDI_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum){
  Caw_printf("midi txcplt\n\r");
  return (USBD_OK);
}




uint8_t USBD_MIDI_GetDeviceState(void){
  return hUsbDevice.dev_state;
}

uint8_t USBD_MIDI_GetState(void){
  USBD_MIDI_HandleTypeDef *hmidi = hUsbDevice.pClassData_MIDI;
  return hmidi->state;
}

uint8_t USBD_MIDI_SendReport(uint8_t *report, uint16_t len){
  USBD_MIDI_HandleTypeDef *hmidi = hUsbDevice.pClassData_MIDI;

  if (hUsbDevice.dev_state == USBD_STATE_CONFIGURED){
    if(hmidi->state == MIDI_IDLE){
      hmidi->state = MIDI_BUSY;
      USBD_LL_Transmit (&hUsbDevice, MIDI_IN_EP, report, len);
    }
  }
  return USBD_OK;
}
