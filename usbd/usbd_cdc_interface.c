/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Src/usbd_cdc_interface.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    14-April-2017
  * @brief   Source file for USBD CDC interface
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_main.h"

#include "../ll/debug_usart.h" // debug printing

// Private define
#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048

// Private vars
USBD_CDC_LineCodingTypeDef LineCoding = { 115200 // baud rate
                                        , 0x00   // stop bits-1
                                        , 0x00   // parity - none
                                        , 0x08   // nb. of bits 8
                                        };

uint8_t UserRxBuffer[APP_RX_DATA_SIZE];
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];
uint32_t BuffLength;
uint32_t UserTxBufPtrIn  = 0;
uint32_t UserTxBufPtrOut = 0;

TIM_HandleTypeDef  TimHandle;

extern USBD_HandleTypeDef  USBD_Device;

// Private declarations
static int8_t CDC_Itf_Init(void);
static int8_t CDC_Itf_DeInit(void);
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive(uint8_t* pbuf, uint32_t *Len);

static void ComPort_Config(void);
static void TIM_Config(void);

USBD_CDC_ItfTypeDef USBD_CDC_fops = { CDC_Itf_Init
                                    , CDC_Itf_DeInit
                                    , CDC_Itf_Control
                                    , CDC_Itf_Receive
                                    };

/* Private functions ---------------------------------------------------------*/
static int8_t CDC_Itf_Init(void)
{
    TIM_Config();
    if( HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK ){ U_PrintLn("!tim_start"); }

    USBD_CDC_SetTxBuffer(&USBD_Device, UserTxBuffer, 0);
    USBD_CDC_SetRxBuffer(&USBD_Device, UserRxBuffer);

    return (USBD_OK);
}

static int8_t CDC_Itf_DeInit(void)
{
    U_PrintLn("TODO: CDC_Itf_DeInit");
    return (USBD_OK);
}

static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
    static int8_t lcoding = 3; // just a dumb 'ready print'

    // Most of this is unimplemented!
    switch( cmd ){
        case CDC_SEND_ENCAPSULATED_COMMAND: U_PrintLn("itf:send_cmd");     break;
        case CDC_GET_ENCAPSULATED_RESPONSE: U_PrintLn("itf:get_response"); break;
        case CDC_SET_COMM_FEATURE:          U_PrintLn("itf:set_feature");  break;
        case CDC_GET_COMM_FEATURE:          U_PrintLn("itf:get_feature");  break;
        case CDC_CLEAR_COMM_FEATURE:        U_PrintLn("itf:clr_feature");  break;
        case CDC_SET_LINE_CODING:
            LineCoding.bitrate    = (uint32_t)(  pbuf[0]
                                              | (pbuf[1] << 8)
                                              | (pbuf[2] << 16)
                                              | (pbuf[3] << 24)
                                              );
            LineCoding.format     = pbuf[4];
            LineCoding.paritytype = pbuf[5];
            LineCoding.datatype   = pbuf[6];
            // Set the new configuration
            ComPort_Config();
            if( lcoding ){ lcoding--; }
            else{
                U_PrintLn("Dialing...");
                USB_tx_enqueue( (uint8_t*)"\n\nDialing...\n\0", 14 );
            }

            break;
        case CDC_GET_LINE_CODING: U_PrintLn("itf:g_line_coding");
            pbuf[0] = (uint8_t)(LineCoding.bitrate);
            pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
            pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
            pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
            pbuf[4] = LineCoding.format;
            pbuf[5] = LineCoding.paritytype;
            pbuf[6] = LineCoding.datatype;
            break;
        case CDC_SET_CONTROL_LINE_STATE: U_PrintLn("itf:s_ctrl_state"); break;
        case CDC_SEND_BREAK:             U_PrintLn("itf:send_brk");     break;
        default: break;
    }
    return (USBD_OK);
}

uint8_t tx_has_data = 0;
uint32_t tx_buf_size = 0;
void USB_tx_enqueue( uint8_t* buf, uint32_t len )
{
    tx_has_data = 1;
    if( len > APP_TX_DATA_SIZE ){ len = APP_TX_DATA_SIZE; } // clip length
    memcpy( UserTxBuffer
          , buf
          , len
          );
    tx_buf_size = len;
}
// This could be called directly from the scheduler in main (not TIM)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // THIS IS JUST OVERWRITTEN RN! NO QUEUE!
    if( tx_has_data ){
        USBD_CDC_SetTxBuffer( &USBD_Device
                            , (uint8_t*)UserTxBuffer
                            , tx_buf_size
                            );
        if( USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK ){
            // if succeeded, clear the buffer (else leave it to retry)
            tx_has_data = 0;
            /*UserTxBufPtrOut += buffsize;
            if( UserTxBufPtrOut == APP_RX_DATA_SIZE ){
                UserTxBufPtrOut = 0;
            }*/
        }
    }
}


// RECEIVE

uint32_t rx_buf_size = 0;
uint32_t rx_buf_ptr  = 0;
uint8_t rx_has_data  = 0;
uint8_t USB_rx_has_data( void )
{
    return rx_has_data;
}

static int8_t CDC_Itf_Receive( uint8_t* buf, uint32_t *len )
{
    // enqueue data in the USB_rx buffer
    // TODO: obviously just proof of concept. doesn't have a real queue
    if(*len > 0){
        rx_buf_size += *len;
        rx_has_data  = 1;
    }
    return (USBD_OK);
}
uint8_t USB_rx_dequeue( uint8_t** buf, uint32_t* len )
{
    uint8_t has_data = 0;
    if( (has_data = USB_rx_has_data()) ){
        *buf  = &(UserRxBuffer[rx_buf_ptr]);
        *len = rx_buf_size;
        rx_buf_size = 0; // clear buf
        rx_buf_ptr  = 0; // reset index
        rx_has_data = 0; // no data

        USBD_CDC_SetRxBuffer( &USBD_Device, UserRxBuffer ); // reset buffer
        USBD_CDC_ReceivePacket(&USBD_Device); // Receive the next packet
    }
    return has_data;
}

static void ComPort_Config(void)
{
    /*
    switch( LineCoding.format ){ // Set the Stop bit
        case 0:  UartHandle.Init.StopBits = UART_STOPBITS_1; break;
        case 2:  UartHandle.Init.StopBits = UART_STOPBITS_2; break;
        default: UartHandle.Init.StopBits = UART_STOPBITS_1; break;
    }
  
    switch( LineCoding.paritytype ){ // Set the Parity bit
        case 0:  UartHandle.Init.Parity = UART_PARITY_NONE; break;
        case 1:  UartHandle.Init.Parity = UART_PARITY_ODD;  break;
        case 2:  UartHandle.Init.Parity = UART_PARITY_EVEN; break;
        default: UartHandle.Init.Parity = UART_PARITY_NONE; break;
    }
  
    //set the data type : only 8bits and 9bits is supported
    switch( LineCoding.datatype ){ // Only 8b and 9b supported
        case 0x07: // With this configuration a parity (Even or Odd) must be set
            UartHandle.Init.WordLength = UART_WORDLENGTH_8B; break;
        case 0x08:
            UartHandle.Init.WordLength = (UartHandle.Init.Parity == UART_PARITY_NONE)
                                            ? UART_WORDLENGTH_8B
                                            : UART_WORDLENGTH_9B; break;
        default: UartHandle.Init.WordLength = UART_WORDLENGTH_8B; break;
    }
  
    UartHandle.Init.BaudRate     = LineCoding.bitrate;
    */

    // Now restart reception
}

static void TIM_Config(void)
{  
    // Set TIMu instance
    TimHandle.Instance = TIMu;
  
    TIMu_CLK_ENABLE();
    // Initialize TIM3 peripheral as follow:
    //     + Period = 10000 - 1
    //     + Prescaler = ((SystemCoreClock/2)/10000) - 1
    //     + ClockDivision = 0
    //     + Counter direction = Up
    //
    TimHandle.Init.Period = (CDC_POLLING_INTERVAL*1000) - 1;
    TimHandle.Init.Prescaler = 84-1;
    TimHandle.Init.ClockDivision = 0;
    TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if( HAL_TIM_Base_Init(&TimHandle) != HAL_OK ){ U_PrintLn("!tim-base"); }

    HAL_NVIC_SetPriority( TIMu_IRQn, 6, 0 );
    HAL_NVIC_EnableIRQ( TIMu_IRQn );
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
