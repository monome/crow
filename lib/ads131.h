#pragma once

#include <stm32f4xx.h>

#include "stm32f4xx_hal_conf.h"

// Definition for SPIa clock resources
#define SPIa                             SPI1
#define SPIa_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIa_DMAx_CLK_ENABLE()           __HAL_RCC_DMA2_CLK_ENABLE()
#define SPIa_NSS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIa_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIa_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIa_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIa_MCLK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define SPIa_FORCE_RESET()               __HAL_RCC_SPI1_FORCE_RESET()
#define SPIa_RELEASE_RESET()             __HAL_RCC_SPI1_RELEASE_RESET()

// Definition for SPIa Pins
#define SPIa_NSS_PIN                     GPIO_PIN_4
#define SPIa_NSS_GPIO_PORT               GPIOA
#define SPIa_SCK_PIN                     GPIO_PIN_5
#define SPIa_SCK_GPIO_PORT               GPIOA
#define SPIa_SCK_AF                      GPIO_AF5_SPI1
#define SPIa_MISO_PIN                    GPIO_PIN_6
#define SPIa_MISO_GPIO_PORT              GPIOA
#define SPIa_MISO_AF                     GPIO_AF5_SPI1
#define SPIa_MOSI_PIN                    GPIO_PIN_7
#define SPIa_MOSI_GPIO_PORT              GPIOA
#define SPIa_MOSI_AF                     GPIO_AF5_SPI1

#define SPIa_MCLK_PIN                    GPIO_PIN_0
#define SPIa_MCLK_GPIO_PORT              GPIOB
#define SPIa_MCLK_AF                     GPIO_AF5_SPI1 // this is PWM AF

// Definition for SPIa's DMA
#define SPIa_TX_DMA_CHANNEL              DMA_CHANNEL_3
#define SPIa_TX_DMA_STREAM               DMA2_Stream3
#define SPIa_RX_DMA_CHANNEL              DMA_CHANNEL_3
#define SPIa_RX_DMA_STREAM               DMA2_Stream0

// Definition for SPIa's NVIC
#define SPIa_IRQn                        SPI1_IRQn
#define SPIa_IRQHandler                  SPI1_IRQHandler
#define SPIa_DMA_TX_IRQn                 DMA2_Stream3_IRQn
#define SPIa_DMA_RX_IRQn                 DMA2_Stream0_IRQn
#define SPIa_DMA_TX_IRQHandler           DMA2_Stream3_IRQHandler
#define SPIa_DMA_RX_IRQHandler           DMA2_Stream0_IRQHandler

void ADC_Init(void);

int32_t ADC_Get( uint8_t channel );

void SPIa_DMA_RX_IRQHandler(void);
void SPIa_DMA_TX_IRQHandler(void);
void SPIa_IRQHandler(void);

void ADC_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
void ADC_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);

void ADC_SPI_MspInit(SPI_HandleTypeDef *hspi);
void ADC_SPI_MspDeInit(SPI_HandleTypeDef *hspi);
