#pragma once

#include <stm32f7xx.h>
#include "interrupts.h" // MIDI_IRQPriority

#define MIDI_GPIO_RCC()         __HAL_RCC_GPIOA_CLK_ENABLE()
#define MIDI_UART_RCC()         __HAL_RCC_UART4_CLK_ENABLE()
#define MIDI_DMA_CLK_ENABLE()   __HAL_RCC_DMA1_CLK_ENABLE()

#define MIDIx_FORCE_RESET()     __UART4_FORCE_RESET()
#define MIDIx_RELEASE_RESET()   __UART4_RELEASE_RESET()


#define MIDI_baud               31250
#define MIDIx                   UART4
#define MIDI_AF                 GPIO_AF8_UART4
#define MIDI_GPIO               GPIOA
#define MIDI_RXPIN              GPIO_PIN_1

#define MIDIx_RX_DMA_STREAM     DMA1_Stream2
#define MIDIx_RX_DMA_CHANNEL    DMA_CHANNEL_4

#define MIDIx_DMA_RX_IRQn       DMA1_Stream2_IRQn
#define MIDI_ll_DMA_RX_IRQHandler DMA1_Stream2_IRQHandler

#define MIDIx_IRQn                  UART4_IRQn
#define MIDI_ll_IRQHandler          UART4_IRQHandler
#define MIDIx_IRQPriority           MIDI_IRQPriority
#define MIDIx_IRQSubPriority        0
#define MIDIx_DMA_IRQPriority       MIDI_IRQPriority
#define MIDIx_DMA_IRQSubPriority    1

#define DBG_UART_TIMEOUT    0x4000 /* a long time */

void MIDI_ll_Init( void(*rx_handler)(uint8_t*)
                 , void(*error_handler)(void) );
void MIDI_ll_DeInit(void);

int MIDI_ll_Rx( int ix, int count );

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart );
void HAL_UART_ErrorCallback( UART_HandleTypeDef *huart );
void MIDI_ll_IRQHandler( void );
