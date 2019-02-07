#pragma once

#include <stm32f7xx.h>

#include "stm32f7xx_hal_conf.h"
#include "interrupts.h" // DEBUG_IRQPriority

#define DBG_USART_GPIO_RCC()	__HAL_RCC_GPIOC_CLK_ENABLE()
#define DBG_USART_USART_RCC()	__HAL_RCC_USART3_CLK_ENABLE()
#define DBG_DMAx_CLK_ENABLE()	__HAL_RCC_DMA1_CLK_ENABLE()

#define DBG_USART_baud			115200
#define DBG_USARTx 				USART3
#define DBG_USART_AF			GPIO_AF7_USART3
#define DBG_USART_GPIO			GPIOC
#define DBG_USART_TXPIN			GPIO_PIN_10
#define DBG_USART_RXPIN			GPIO_PIN_11

// Definition for USARTx's DMA
#define USARTx_TX_DMA_STREAM	DMA1_Stream3
#define USARTx_RX_DMA_STREAM	DMA1_Stream1
#define USARTx_TX_DMA_CHANNEL	DMA_CHANNEL_4
#define USARTx_RX_DMA_CHANNEL	DMA_CHANNEL_4

#define USARTx_DMA_TX_IRQn		DMA1_Stream3_IRQn
#define USARTx_DMA_RX_IRQn		DMA1_Stream1_IRQn
#define USARTx_DMA_TX_IRQHandler	DMA1_Stream3_IRQHandler
#define USARTx_DMA_RX_IRQHandler	DMA1_Stream1_IRQHandler

#define USARTx_IRQn				    USART3_IRQn
#define USARTx_IRQHandler		    USART3_IRQHandler
#define USARTx_IRQPriority          DEBUG_IRQPriority
#define USARTx_IRQSubPriority       0
#define USARTx_DMA_IRQPriority      DEBUG_IRQPriority
#define USARTx_DMA_TXIRQSubPriority 2
#define USARTx_DMA_RXIRQSubPriority 1

#define DBG_USART_TIMEOUT	0x4000 /* a long time */

// Setup functions & DMA/IT Handlers
void Debug_USART_Init(void);
void Debug_USART_DeInit(void);
void USARTx_DMA_TX_IRQHandler(void);
void HAL_USART_TxCpltCallback(USART_HandleTypeDef *husart);
void USARTx_IRQHandler(void);

// Communication Fns
void U_PrintNow(void);
void U_Print(char* s, int len);
