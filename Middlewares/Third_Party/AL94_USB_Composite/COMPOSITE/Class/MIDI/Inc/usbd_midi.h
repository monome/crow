#pragma once

#include "usbd_ioreq.h"
#include "AL94.I-CUBE-USBD-COMPOSITE_conf.h"

#define MIDI_STR_DESC             "STM32 MIDI DEVICE"

#define MIDI_IN_PORTS_NUM              0x01
#define MIDI_OUT_PORTS_NUM             0x01

// #define MIDI_EPIN_ADDR                 0x81
#define MIDI_EPIN_SIZE                 0x40

// #define MIDI_EPOUT_ADDR                0x01
#define MIDI_EPOUT_SIZE                0x40

#define USB_MIDI_CLASS_DESC_SHIFT      18
#define USB_MIDI_DESC_SIZE             7
#define USB_MIDI_REPORT_DESC_SIZE      (MIDI_IN_PORTS_NUM * 16 + MIDI_OUT_PORTS_NUM * 16 + 33)
#define USB_MIDI_CONFIG_DESC_SIZE      (USB_MIDI_REPORT_DESC_SIZE + USB_MIDI_CLASS_DESC_SHIFT)

#define MIDI_DESCRIPTOR_TYPE           0x21
  
#define MIDI_REQ_SET_PROTOCOL          0x0B
#define MIDI_REQ_GET_PROTOCOL          0x03

#define MIDI_REQ_SET_IDLE              0x0A
#define MIDI_REQ_GET_IDLE              0x02

#define MIDI_REQ_SET_REPORT            0x09
#define MIDI_REQ_GET_REPORT            0x01

#define MIDI_JACK_1    0x01
#define MIDI_JACK_2    0x02
#define MIDI_JACK_3    0x03
#define MIDI_JACK_4    0x04
#define MIDI_JACK_5    0x05
#define MIDI_JACK_6    0x06
#define MIDI_JACK_7    0x07
#define MIDI_JACK_8    0x08
#define MIDI_JACK_9    0x09
#define MIDI_JACK_10   0x0a
#define MIDI_JACK_11   0x0b
#define MIDI_JACK_12   0x0c
#define MIDI_JACK_13   0x0d
#define MIDI_JACK_14   0x0e
#define MIDI_JACK_15   0x0f
#define MIDI_JACK_16   0x10
#define MIDI_JACK_17   (MIDI_IN_PORTS_NUM * 2 + 0x01)
#define MIDI_JACK_18   (MIDI_IN_PORTS_NUM * 2 + 0x02)
#define MIDI_JACK_19   (MIDI_IN_PORTS_NUM * 2 + 0x03)
#define MIDI_JACK_20   (MIDI_IN_PORTS_NUM * 2 + 0x04)
#define MIDI_JACK_21   (MIDI_IN_PORTS_NUM * 2 + 0x05)
#define MIDI_JACK_22   (MIDI_IN_PORTS_NUM * 2 + 0x06)
#define MIDI_JACK_23   (MIDI_IN_PORTS_NUM * 2 + 0x07)
#define MIDI_JACK_24   (MIDI_IN_PORTS_NUM * 2 + 0x08)
#define MIDI_JACK_25   (MIDI_IN_PORTS_NUM * 2 + 0x09)
#define MIDI_JACK_26   (MIDI_IN_PORTS_NUM * 2 + 0x0a)
#define MIDI_JACK_27   (MIDI_IN_PORTS_NUM * 2 + 0x0b)
#define MIDI_JACK_28   (MIDI_IN_PORTS_NUM * 2 + 0x0c)
#define MIDI_JACK_29   (MIDI_IN_PORTS_NUM * 2 + 0x0d)
#define MIDI_JACK_30   (MIDI_IN_PORTS_NUM * 2 + 0x0e)
#define MIDI_JACK_31   (MIDI_IN_PORTS_NUM * 2 + 0x0f)
#define MIDI_JACK_32   (MIDI_IN_PORTS_NUM * 2 + 0x10)

typedef enum{
  MIDI_IDLE = 0,
  MIDI_BUSY,
} MIDI_StateTypeDef; 

typedef struct _USBD_MIDI_Itf{
  int8_t (*Init)(void);
  int8_t (*DeInit)(void);
  int8_t (*Control)(uint8_t cmd, uint8_t *pbuf, uint16_t length);
  int8_t (*Receive)(uint8_t *Buf, uint32_t *Len);
  int8_t (*TransmitCplt)(uint8_t *Buf, uint32_t *Len, uint8_t epnum);
} USBD_MIDI_ItfTypeDef;

typedef struct{
  uint32_t          Protocol;   
  uint32_t          IdleState;  
  uint32_t          AltSetting;
  MIDI_StateTypeDef state;
  uint8_t *RxBuffer;
  uint32_t RxLength;
} USBD_MIDI_HandleTypeDef;

extern USBD_ClassTypeDef USBD_MIDI;

extern USBD_MIDI_HandleTypeDef MIDI_Class_Data;

extern uint8_t MIDI_IN_EP;
extern uint8_t MIDI_OUT_EP;
extern uint8_t MIDI_ITF_NBR;
extern uint8_t MIDI_STR_DESC_IDX;

uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops);
uint8_t USBD_MIDI_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t* pbuf);

void USBD_Update_MIDI_DESC(uint8_t *desc, uint8_t itf_no, uint8_t in_ep, uint8_t out_ep, uint8_t str_idx);
