#pragma once

#include <stm32f4xx.h>

#include "stm32f4xx_hal_conf.h"

// Definition for SPId clock resources
#define SPId                             SPI2
#define SPId_CLK_ENABLE()                __HAL_RCC_SPI2_CLK_ENABLE()
#define SPId_DMAx_CLK_ENABLE()           __HAL_RCC_DMA1_CLK_ENABLE()
#define SPId_NSS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPId_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPId_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPId_NRST_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define SPId_FORCE_RESET()               __HAL_RCC_SPI2_FORCE_RESET()
#define SPId_RELEASE_RESET()             __HAL_RCC_SPI2_RELEASE_RESET()

// Definition for SPId Pins
#define SPId_NSS_PIN                     GPIO_PIN_12
#define SPId_NSS_GPIO_PORT               GPIOB
#define SPId_SCK_PIN                     GPIO_PIN_13
#define SPId_SCK_GPIO_PORT               GPIOB
#define SPId_SCK_AF                      GPIO_AF5_SPI2
#define SPId_MOSI_PIN                    GPIO_PIN_15
#define SPId_MOSI_GPIO_PORT              GPIOB
#define SPId_MOSI_AF                     GPIO_AF5_SPI2
#define SPId_NRST_PIN                    GPIO_PIN_14
#define SPId_NRST_GPIO_PORT              GPIOB

// Definition for SPId's DMA
#define SPId_TX_DMA_CHANNEL              DMA_CHANNEL_0
#define SPId_TX_DMA_STREAM               DMA1_Stream4

// Definition for SPId's NVIC
#define SPId_IRQn                        SPI2_IRQn
#define SPId_IRQHandler                  SPI2_IRQHandler
#define SPId_DMA_TX_IRQn                 DMA1_Stream4_IRQn
#define SPId_DMA_TX_IRQHandler           DMA1_Stream4_IRQHandler

// dac8565 defines
#define DAC8565_SET_ONE     0b00010000
#define DAC8565_PREP_ONE    0x0
#define DAC8565_SET_ALL     0b00110100
#define DAC8565_REFRESH_ALL 0b00110000

#define DAC_ALL_CHANNELS    ((int8_t)-1)
#define DAC_ZERO_VOLTS      ((uint16_t)(((uint32_t)0xFFFF * 2)/3))

void DAC_Init(void);

void DAC_Set( int8_t channel, uint16_t value );

void SPId_DMA_TX_IRQHandler(void);
void SPId_IRQHandler(void);
