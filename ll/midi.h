#pragma once

#include <stm32f7xx.h>
#include "debug_usart.h"

#include "stm32f7xx_hal_conf.h"

#define MIDI_GPIO_RCC()	        __HAL_RCC_GPIOB_CLK_ENABLE()
#define MIDI_USART_RCC()	    __HAL_RCC_USART1_CLK_ENABLE()
#define MIDI_DMA_CLK_ENABLE()	__HAL_RCC_DMA2_CLK_ENABLE()

#define MIDI_baud			    31250
#define MIDIx 				    USART1
#define MIDI_AF			        GPIO_AF7_USART1
#define MIDI_GPIO			    GPIOB
#define MIDI_RXPIN			    GPIO_PIN_7

// Definition for USARTx's DMA
#define MIDIx_RX_DMA_STREAM	    DMA2_Stream2 // or 5
#define MIDIx_RX_DMA_CHANNEL	DMA_CHANNEL_4

#define MIDIx_DMA_RX_IRQn		DMA2_Stream2_IRQn
#define MIDIx_DMA_RX_IRQHandler	DMA2_Stream2_IRQHandler

#define MIDIx_IRQn				    USART1_IRQn
#define MIDIx_IRQHandler		    USART1_IRQHandler
#define MIDIx_IRQPriority           6
#define MIDIx_IRQSubPriority        0
#define MIDIx_DMA_IRQPriority       6
#define MIDIx_DMA_IRQSubPriority    1


#define DBG_USART_TIMEOUT	0x4000 /* a long time */

void MIDI_Init(void);
void MIDI_DeInit(void);

void MIDI_MspInit(USART_HandleTypeDef *hu );

void USARTx_DMA_RX_IRQHandler(void);
void HAL_USART_RxCpltCallback(USART_HandleTypeDef *husart);
//void USARTx_IRQHandler(void);
