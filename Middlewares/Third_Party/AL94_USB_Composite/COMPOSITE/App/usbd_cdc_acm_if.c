#include "usbd_cdc_acm_if.h"
#include "usbd_cdc_acm.h"
#include "ll/timers.h"
#include "usbd_conf.h"
#include "ll/interrupts.h"

#define APP_RX_DATA_SIZE 128
#define APP_TX_DATA_SIZE 128
#define CONNECTION_DELAY  500 // millisecond delay before sending buffer after connect

uint8_t RX_Buffer[NUMBER_OF_CDC][APP_RX_DATA_SIZE];
uint8_t TX_Buffer[NUMBER_OF_CDC][APP_TX_DATA_SIZE];

USBD_CDC_ACM_LineCodingTypeDef Line_Coding[NUMBER_OF_CDC];

uint32_t Write_Index[NUMBER_OF_CDC]; /* keep track of received data over UART */
uint32_t Read_Index[NUMBER_OF_CDC];  /* keep track of sent data to USB */

static int timer_index = -1;

extern USBD_HandleTypeDef hUsbDevice;

static int8_t CDC_Init(uint8_t cdc_ch);
static int8_t CDC_DeInit(uint8_t cdc_ch);
static int8_t CDC_Control(uint8_t cdc_ch, uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive(uint8_t cdc_ch, uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt(uint8_t cdc_ch, uint8_t *Buf, uint32_t *Len, uint8_t epnum);

static void USB_Timer_Callback(int count);

USBD_CDC_ACM_ItfTypeDef USBD_CDC_ACM_fops = {CDC_Init,
                                             CDC_DeInit,
                                             CDC_Control,
                                             CDC_Receive,
                                             CDC_TransmitCplt};

#include "ll/status_led.h" // increase LED blink speed when USB connected
static int timerdelay = 0;
static int8_t CDC_Init(uint8_t cdc_ch){
  USBD_CDC_SetRxBuffer(cdc_ch, &hUsbDevice, RX_Buffer[cdc_ch]);

  // set the timerdelay, so the TIM can be started but *not* send for
  // the first 100ms to solve ECHO issue on norns. see #137
  timerdelay = CONNECTION_DELAY / CDC_POLLING_INTERVAL;

  if(timer_index >= 0){
    Timer_Set_Params( timer_index, CDC_POLLING_INTERVAL/1000.0 );
    Timer_Priority( timer_index, USB_IRQPriority );
    Timer_Start( timer_index, &USB_Timer_Callback );
  } else {
    printf("ERROR: timer-index not set\n\r");
  }

  printf("USB_CDC_Init\n");
  status_led_fast(LED_FAST);

  return (USBD_OK);
}

static int8_t CDC_DeInit(uint8_t cdc_ch){
  Timer_Stop( timer_index );
  printf("USB_DeInit\n");
  status_led_fast(LED_SLOW);
  return (USBD_OK);
}

void CDC_Set_Timer_Index(int tix){
  timer_index = tix;
}

void CDC_clear_buffers(void){
  for( int i=0; i<APP_RX_DATA_SIZE; i++ ){ RX_Buffer[0][i] = 0; }
  for( int i=0; i<APP_TX_DATA_SIZE; i++ ){ TX_Buffer[0][i] = 0; }
  Write_Index[0]  = 0;
  Read_Index[0]  = 0;
  USBD_CDC_SetRxBuffer(0, &hUsbDevice, RX_Buffer[0]);
  USBD_CDC_SetTxBuffer(0, &hUsbDevice, TX_Buffer[0], 0);
}

static int8_t CDC_Control(uint8_t cdc_ch, uint8_t cmd, uint8_t *pbuf, uint16_t length){
  switch (cmd){
  case CDC_SEND_ENCAPSULATED_COMMAND:
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    break;

  case CDC_SET_COMM_FEATURE:
    break;

  case CDC_GET_COMM_FEATURE:
    break;

  case CDC_CLEAR_COMM_FEATURE:
    break;

    /*******************************************************************************/
    /* Line Coding Structure                                                       */
    /*-----------------------------------------------------------------------------*/
    /* Offset | Field       | Size | Value  | Description                          */
    /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
    /* 4      | bCharFormat |   1  | Number | Stop bits                            */
    /*                                        0 - 1 Stop bit                       */
    /*                                        1 - 1.5 Stop bits                    */
    /*                                        2 - 2 Stop bits                      */
    /* 5      | bParityType |  1   | Number | Parity                               */
    /*                                        0 - None                             */
    /*                                        1 - Odd                              */
    /*                                        2 - Even                             */
    /*                                        3 - Mark                             */
    /*                                        4 - Space                            */
    /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
    /*******************************************************************************/
  case CDC_SET_LINE_CODING:
    Line_Coding[cdc_ch].bitrate = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |
                                             (pbuf[2] << 16) | (pbuf[3] << 24));
    Line_Coding[cdc_ch].format = pbuf[4];
    Line_Coding[cdc_ch].paritytype = pbuf[5];
    Line_Coding[cdc_ch].datatype = pbuf[6];

    //Change_UART_Setting(cdc_ch);
    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t)(Line_Coding[cdc_ch].bitrate);
    pbuf[1] = (uint8_t)(Line_Coding[cdc_ch].bitrate >> 8);
    pbuf[2] = (uint8_t)(Line_Coding[cdc_ch].bitrate >> 16);
    pbuf[3] = (uint8_t)(Line_Coding[cdc_ch].bitrate >> 24);
    pbuf[4] = Line_Coding[cdc_ch].format;
    pbuf[5] = Line_Coding[cdc_ch].paritytype;
    pbuf[6] = Line_Coding[cdc_ch].datatype;
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    break;

  case CDC_SEND_BREAK:
    break;

  default:
    break;
  }

  return (USBD_OK);
}

static int8_t CDC_Receive(uint8_t cdc_ch, uint8_t *Buf, uint32_t *Len){
  uint32_t length = *Len;
  if( (Write_Index[cdc_ch] + length) >= APP_RX_DATA_SIZE ){
      length = APP_RX_DATA_SIZE - Write_Index[cdc_ch]; // stop buffer overflow
  }
  memcpy( &RX_Buffer[cdc_ch][Write_Index[cdc_ch]]
        , Buf
        , length
        );
  Write_Index[cdc_ch] += length;
  return USBD_OK;
  //HAL_UART_Transmit_DMA(CDC_CH_To_UART_Handle(cdc_ch), Buf, *Len);
  // CDC_Transmit(cdc_ch, Buf, *Len); // echo back on same channel

  // USBD_CDC_SetRxBuffer(cdc_ch, &hUsbDevice, &Buf[0]);
  // USBD_CDC_ReceivePacket(cdc_ch, &hUsbDevice);
  // return (USBD_OK);
}

static int8_t CDC_TransmitCplt(uint8_t cdc_ch, uint8_t *Buf, uint32_t *Len, uint8_t epnum){
  // TODO. use this to rotate circular buffer etc.
  return (USBD_OK);
}

uint8_t CDC_Transmit_Enqueue(uint8_t ch, uint8_t *Buf, uint16_t Len){
  // WARNING: UserTxDataLen having room doesn't necessarily mean
  // the buffer is junk data. There can still be an ongoing transfer
  // using the end of the buffer (usually just the last few bytes).
  // Check USB_tx_is_ready()==1 if you need to be sure no data is lost.
  // especially if *streaming* data to the CDC.
  if( (Read_Index[ch] + Len) >= APP_TX_DATA_SIZE ){
      Len = APP_TX_DATA_SIZE - Read_Index[ch]; // stop buffer overflow
  }
  if( Len == 0 ){
      // FIXME? Likely means we're trying to TX when no usb device connected
      //printf("TxBuf full\n"); // TODO memcpy will still run (can rm this warning)
  }
  memcpy( &TX_Buffer[ch][Read_Index[ch]]
        , Buf
        , Len
        );
  Read_Index[ch] += Len;

  return 0;
}

// UNUSED!!!
uint8_t CDC_Transmit(uint8_t ch, uint8_t *Buf, uint16_t Len){
  uint8_t result = USBD_OK;

  extern USBD_CDC_ACM_HandleTypeDef CDC_ACM_Class_Data[];
  USBD_CDC_ACM_HandleTypeDef *hcdc = NULL;
  hcdc = &CDC_ACM_Class_Data[ch];

  if (hcdc->TxState != 0)
  {
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(ch, &hUsbDevice, Buf, Len);
  result = USBD_CDC_TransmitPacket(ch, &hUsbDevice);
  /* USER CODE END 7 */
  return result;
}

int CDC_Transmit_Is_Ready(void){
  extern USBD_CDC_ACM_HandleTypeDef CDC_ACM_Class_Data[];
  USBD_CDC_ACM_HandleTypeDef *hcdc = NULL;
  hcdc = &CDC_ACM_Class_Data[0];

  return (Read_Index[0] == 0)  // ensure the bufer is empty
      && (hcdc->TxState == 0); // CDC has finished active tx
}

size_t CDC_Transmit_Space(void){
  return (size_t)(APP_TX_DATA_SIZE - Read_Index[0]);
}

int CDC_Receive_Dequeue_LOCK(uint8_t ch, uint8_t** buf, uint32_t* len){
  if(Write_Index[ch]){ // non-zero means data is present
    *buf = RX_Buffer[ch];
    *len = Write_Index[ch];
    Write_Index[ch] = 0; // reset rx array
    return 1;
  }
  return 0; 
}

void CDC_Receive_Dequeue_UNLOCK(uint8_t ch){
  if(!Write_Index[ch]){ // zero means data has been processed
// uint32_t old_primask = __get_PRIMASK();
// __disable_irq();
    USBD_CDC_ReceivePacket(ch, &hUsbDevice); // Receive the next packet
// __set_PRIMASK( old_primask );
  } else {
    printf("data in rx_queue means something's wrong.\n");
  }
}

// interrupt sends out any queued data
static void USB_Timer_Callback(int count){
    // here we NOP the first 100ms of timer clicks
    // see PR #137. solves ECHO issue on norns.
    if( timerdelay ){ timerdelay--; return; }
    if(Read_Index[0]){
        if( Read_Index[0] >= APP_TX_DATA_SIZE ){
            //printf("overflow %i\n",(int)Read_Index[0]);
            Read_Index[0] = APP_TX_DATA_SIZE;
        }
        USBD_CDC_SetTxBuffer( 0, &hUsbDevice
                            , TX_Buffer[0]
                            , Read_Index[0] 
                            );
        int error = USBD_OK;
        if( (error = USBD_CDC_TransmitPacket(0, &hUsbDevice)) ){
            // This means the buffer is full & hasn't been read
            printf("CDC_tx failed %i\n", error);
        } else {
            Read_Index[0] = 0; // only clear data if no error
        }
    }
}
