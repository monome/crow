#pragma once

#include <stm32f4xx.h>

#include "stm32f4xx_hal_conf.h"

#define DBG_USART_GPIO_RCC()	__HAL_RCC_GPIOB_CLK_ENABLE()
#define DBG_USART_USART_RCC()	__HAL_RCC_USART1_CLK_ENABLE()
#define DBG_DMAx_CLK_ENABLE()	__HAL_RCC_DMA2_CLK_ENABLE()

#define DBG_USART_baud			115200
#define DBG_USARTx 				USART1
#define DBG_USART_AF			GPIO_AF7_USART1
#define DBG_USART_GPIO			GPIOB
#define DBG_USART_TXPIN			GPIO_PIN_6
#define DBG_USART_RXPIN			GPIO_PIN_7

// Definition for USARTx's DMA
#define USARTx_TX_DMA_STREAM	DMA2_Stream7
#define USARTx_RX_DMA_STREAM	DMA2_Stream2
#define USARTx_TX_DMA_CHANNEL	DMA_CHANNEL_4
#define USARTx_RX_DMA_CHANNEL	DMA_CHANNEL_4

#define USARTx_DMA_TX_IRQn		DMA2_Stream7_IRQn
#define USARTx_DMA_RX_IRQn		DMA2_Stream2_IRQn
#define USARTx_DMA_TX_IRQHandler	DMA2_Stream7_IRQHandler
#define USARTx_DMA_RX_IRQHandler	DMA2_Stream2_IRQHandler

#define USARTx_IRQn				USART1_IRQn
#define USARTx_IRQHandler		USART1_IRQHandler

#define DBG_USART_TIMEOUT	0x4000 /* a long time */

// Setup functions & DMA/IT Handlers
void Debug_USART_Init(void);
void Debug_USART_DeInit(void);
void USARTx_DMA_TX_IRQHandler(void);
void USARTx_DMA_RX_IRQHandler(void);
void HAL_USART_TxCpltCallback(USART_HandleTypeDef *husart);
void HAL_USART_RxCpltCallback(USART_HandleTypeDef *husart);
void USARTx_IRQHandler(void);

// Communication Fns
void Debug_USART_tryprint(void);
void Debug_USART_printf(char* s);
void Debug_USART_putn(uint32_t n);
void Debug_USART_putn8(uint8_t n);

// Next Gen Fn Calls
void DB_print_var(char* name, uint32_t n, uint8_t ret_flag);
