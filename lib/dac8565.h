#pragma once

#include <stm32f4xx.h>

#include "stm32f4xx_hal_conf.h"

// Definition for SPIx clock resources
#define SPIx                             SPI2
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI2_CLK_ENABLE()
#define SPIx_DMAx_CLK_ENABLE()           __HAL_RCC_DMA1_CLK_ENABLE()
#define SPIx_NSS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_NRST_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define SPIx_FORCE_RESET()               __HAL_RCC_SPI2_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __HAL_RCC_SPI2_RELEASE_RESET()

// Definition for SPIx Pins
#define SPIx_NSS_PIN                     GPIO_PIN_12
#define SPIx_NSS_GPIO_PORT               GPIOB
#define SPIx_SCK_PIN                     GPIO_PIN_13
#define SPIx_SCK_GPIO_PORT               GPIOB
#define SPIx_SCK_AF                      GPIO_AF5_SPI2
#define SPIx_MOSI_PIN                    GPIO_PIN_15
#define SPIx_MOSI_GPIO_PORT              GPIOB
#define SPIx_MOSI_AF                     GPIO_AF5_SPI2
#define SPIx_NRST_PIN                    GPIO_PIN_14
#define SPIx_NRST_GPIO_PORT              GPIOB

// Definition for SPIx's DMA
#define SPIx_TX_DMA_CHANNEL              DMA_CHANNEL_0
#define SPIx_TX_DMA_STREAM               DMA1_Stream4

// Definition for SPIx's NVIC
#define SPIx_IRQn                        SPI2_IRQn
#define SPIx_IRQHandler                  SPI2_IRQHandler
#define SPIx_DMA_TX_IRQn                 DMA1_Stream4_IRQn
#define SPIx_DMA_RX_IRQn                 DMA1_Stream3_IRQn
#define SPIx_DMA_TX_IRQHandler           DMA1_Stream4_IRQHandler
#define SPIx_DMA_RX_IRQHandler           DMA1_Stream3_IRQHandler

// dac8565 defines
#define DAC8565_SET_ONE     0b00010000
#define DAC8565_PREP_ONE    0x0
#define DAC8565_SET_ALL     0b00110100
#define DAC8565_REFRESH_ALL 0b00110000

#define DAC_ALL_CHANNELS    ((int8_t)-1)
#define DAC_ZERO_VOLTS      ((uint16_t)(((uint32_t)0xFFFF * 2)/3))

void DAC_Init(void);

void DAC_Set( int8_t channel, uint16_t value );

void SPIx_DMA_RX_IRQHandler(void);
void SPIx_DMA_TX_IRQHandler(void);
void SPIx_IRQHandler(void);
