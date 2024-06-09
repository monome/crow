#include "usbh_main.h" // USBH_CDC_CLASS

/*
USBH_ClassTypeDef  CDC_Class =
{
  "CDC",
  USB_CDC_CLASS,
  USBH_CDC_InterfaceInit,
  USBH_CDC_InterfaceDeInit,
  USBH_CDC_ClassRequest,
  USBH_CDC_Process,
  USBH_CDC_SOFProcess,
  NULL,
};
*/

static USBH_HandleTypeDef hUSBHost;
// CDC_ApplicationTypeDef Appli_state = APPLICATION_IDLE;


#define TX_BUFF_SIZE 0x400 // 1kB
static uint8_t CDC_TX_Buffer[TX_BUFF_SIZE];
static size_t CDC_TX_Buffer_Count = 0;

#define RX_BUFF_SIZE 0x400 // 1kB
static uint8_t CDC_RX_Buffer[RX_BUFF_SIZE];
static uint8_t CDC_RX_Buffer_TMP[RX_BUFF_SIZE];
static size_t CDC_RX_Buffer_Count = 0;

static int is_connected = 0;


static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);
static void USBHost_Start_Reception(void);
void GetDefaultConfiguration(void);

void USBHost_Init(void){
    USBH_Init(&hUSBHost, USBH_UserProcess, 0); // library function
    USBH_RegisterClass(&hUSBHost, USBH_CDC_CLASS); // library function
    USBH_Start(&hUSBHost); // library function

    // then run usbh_bg_task in the application's main loop
}

// run this in the main loop
int USBHost_BG_Task(void){
    USBH_Process(&hUSBHost); // bg task, lib function
    return CDC_RX_Buffer_Count;
}

// handler called by the system to process state machine of usbh
// this should raise all the useful events for us to handle state changes.
// currently we just use Log prints, but this will allow application level display
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id){
    switch(id){
    case HOST_USER_SELECT_CONFIGURATION:
        // Caw_printf("h_user_select_config\n\r");
        // triggered once enumeration is complete & device is ready
        break;

    case HOST_USER_DISCONNECTION:
        // Caw_printf("h_user_disconnection\n\r");
        // device disconnection has been detected
        // will attempt to reconnect
        // Appli_state = APPLICATION_DISCONNECT;
        is_connected = 0;
        break;

    case HOST_USER_CONNECTION:
        // Caw_printf("h_user_connection\n\r");
        // triggered when the port opening is triggered
        // immediately after this we wait 100ms, then open the control pipes for i/o & enumerate
        // Appli_state = APPLICATION_START;

        // Debug prints VID, PID, mfg, product name etc..
        // next is HOST_USER_CLASS_SELECTED ->
        break;

    case HOST_USER_CLASS_SELECTED:
        // Caw_printf("h_user_class_selected\n\r");
        // a class has been activated
        // next is HOST_USER_CLASS_ACTIVE ->
        break;

    case HOST_USER_CLASS_ACTIVE:
        // Caw_printf("h_user_class_active\n\r");
        GetDefaultConfiguration();
        USBHost_Start_Reception(); // FIXME confirm this is the correct place to start RX
        is_connected = 1;
        // Appli_state = APPLICATION_READY;
        break;

    default:
      break; 
    }
}


/////////////////////////////////////////////////
// user facing functions

void USBHost_Send(uint8_t* data, size_t len){
    if(is_connected){
        CDC_TX_Buffer_Count = (len > TX_BUFF_SIZE) ? TX_BUFF_SIZE : len; // FIXME just truncating long strings
        memcpy(CDC_TX_Buffer, data, CDC_TX_Buffer_Count);
        USBH_CDC_Transmit(&hUSBHost, CDC_TX_Buffer, CDC_TX_Buffer_Count);
    }
}

#include "../lib/caw.h"
void USBH_CDC_TransmitCallback(USBH_HandleTypeDef* phost){
    // uint32_t bytesread;
    // Caw_printf(">> Data sent\n");
}

void USBH_CDC_LineCodingChanged(USBH_HandleTypeDef *phost){
    Caw_printf("line coding changed\n");
}

static void USBHost_Start_Reception(void){
    USBH_CDC_Receive(&hUSBHost, CDC_RX_Buffer, RX_BUFF_SIZE);
}

void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef* phost){
    // TODO use a circular buffer to reduce overflow concerns
    // right now we just memcpy into a 2nd buffer
        // this is safe if we call USBHost_Get_Received() every frame (USBHost_BG_Task())

    // copy received data into 2nd buffer
    CDC_RX_Buffer_Count = USBH_CDC_GetLastReceivedDataSize(phost);
    // Caw_printf("%i, %s\n\r",CDC_RX_Buffer_Count, CDC_RX_Buffer);
    memcpy(CDC_RX_Buffer_TMP, CDC_RX_Buffer, CDC_RX_Buffer_Count);
    CDC_RX_Buffer_TMP[CDC_RX_Buffer_Count] = '\0'; // add null?

    // receive the next usb packet
    USBH_CDC_Receive(&hUSBHost, CDC_RX_Buffer, RX_BUFF_SIZE);
}

// data buffer must be at least RX_BUFF_SIZE large to avoid memory errors
// data is only guaranteed until the next USB frame, so be quick!
size_t USBHost_Get_Received(uint8_t** data){
    size_t len = CDC_RX_Buffer_Count;
    CDC_RX_Buffer_Count = 0; // zero out buffer length to mark it as claimed
    *data = CDC_RX_Buffer_TMP;
    return len;
}

/////////////////////////////////////////////////
// backend handling

extern HCD_HandleTypeDef hhcd;

void OTG_HS_IRQHandler(void){
    HAL_HCD_IRQHandler(&hhcd);
}

void GetDefaultConfiguration(void){
    Caw_printf("GetDefaultConfiguration\n\r");
    // USBH_CDC_GetLineCoding(&hUSBHost, &LineCoding); 
    // DefaultLineCoding = LineCoding;
}
