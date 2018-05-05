#pragma once

#include <stm32f4xx.h>

#include "stm32f4xx_hal_conf.h"

// Defs for MCLK pin, using hardware timer
#define TIMa                             TIM3
#define TIMa_CLK_ENABLE()                __HAL_RCC_TIM3_CLK_ENABLE()

#define TIMa_FORCE_RESET()               __HAL_RCC_TIM3_FORCE_RESET()
#define TIMa_RELEASE_RESET()             __HAL_RCC_TIM3_RELEASE_RESET()

#define TIMa_MCLK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define TIMa_MCLK_PIN                    GPIO_PIN_0
#define TIMa_MCLK_GPIO_PORT              GPIOB
#define TIMa_MCLK_AF                     GPIO_AF2_TIM3 // this is PWM AF
#define TIMa_CHANNEL                     TIM_CHANNEL_3

// Definition for SPIa clock resources
#define SPIa                             SPI1
#define SPIa_CLK_ENABLE()                __HAL_RCC_SPI1_CLK_ENABLE()
#define SPIa_DMAx_CLK_ENABLE()           __HAL_RCC_DMA2_CLK_ENABLE()
#define SPIa_NSS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIa_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIa_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIa_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

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

// ADS131 commands
#define ADS_READY   0xFF02

#define ADS_NULL    0x0
#define ADS_RESET   0x00110000
#define ADS_STANDBY 0x00220000
#define ADS_WAKEUP  0x00330000
#define ADS_LOCK    0x05550000
#define ADS_UNLOCK  0x06550000
//#define ADS_UNLOCK  0x55060000

#define ADS_READ_REG 0x00100000
#define ADS_WRITE_REG 0x01000000


void ADC_Init(void);

//int32_t
void ADC_Get( uint8_t channel );

void SPIa_DMA_RX_IRQHandler(void);
void SPIa_DMA_TX_IRQHandler(void);
void SPIa_IRQHandler(void);

void ADC_SPI_ErrorCallback(SPI_HandleTypeDef *hspi);
void ADC_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);

void ADC_SPI_MspInit(SPI_HandleTypeDef *hspi);
void ADC_SPI_MspDeInit(SPI_HandleTypeDef *hspi);
