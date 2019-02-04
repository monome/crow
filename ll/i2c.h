#pragma once

#include <stm32f7xx.h>
#include "../lib/ii.h" // I2C_RxCpltCallback()
#include "interrupts.h" // I2C_Priority

// #define I2C_TIMING        0x00D00E28 // Rise time = 120ns, Fall time = 25ns
                            // PRESCL
							  // SCLDEL
							   // SDADEL
							    // SCLH
							      // SCLL
#define I2C_TIMING  0x50333090 // based on 400kHz @48MHz i2c clock
#define I2C_MAX_CMD_BYTES 6

#define I2Cx                            I2C1
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1

#define I2Cx_EV_IRQn                    I2C1_EV_IRQn
#define I2Cx_ER_IRQn                    I2C1_ER_IRQn
#define I2Cx_EV_IRQHandler              I2C1_EV_IRQHandler
#define I2Cx_ER_IRQHandler              I2C1_ER_IRQHandler

#define I2Cx_ER_Priority                I2C_Priority
#define I2Cx_ER_SubPriority             3
#define I2Cx_EV_Priority                I2C_Priority
#define I2Cx_EV_SubPriority             4

#define I2C_BUFFER_LEN      32

extern void I2C_RxCpltCallback( uint8_t address, uint8_t cmd, uint8_t* data );

uint8_t I2C_Init( uint8_t address );
void I2C_DeInit( void );

uint8_t I2C_is_boot( void );

// Interrupt handlers for i2c events & errors
void I2Cx_EV_IRQHandler( void );
void I2Cx_ER_IRQHandler( void );

// IRQ callbacks
void HAL_I2C_SlaveRxCpltCallback( I2C_HandleTypeDef* h );
void HAL_I2C_MasterTxCpltCallback( I2C_HandleTypeDef* h );
void HAL_I2C_MasterRxCpltCallback( I2C_HandleTypeDef* h );
void HAL_I2C_ErrorCallback( I2C_HandleTypeDef* h );
void HAL_I2C_ListenCpltCallback( I2C_HandleTypeDef* hi2c );
void HAL_I2C_AddrCallback( I2C_HandleTypeDef* h
	                     , uint8_t            TransferDirection
	                     , uint16_t           AddrMatchCode
	                     );
void I2C_BufferRx( uint8_t* data );
uint8_t I2C_GetAddress( void );
void I2C_SetAddress( uint8_t address );
uint8_t* I2C_PopFollowBuffer( void );
uint8_t I2C_FollowBufferNotEmpty( void );
uint8_t* I2C_PopLeadBuffer( void );
uint8_t I2C_LeadBufferNotEmpty( void );
void I2C_SetTxData( uint8_t* data, uint8_t size );
uint8_t I2C_LeadTx( uint8_t  address
                  , uint8_t* data
                  , uint8_t  size
                  );
uint8_t I2C_LeadRx( uint8_t  address
                  , uint8_t* data
                  , uint8_t  size
                  , uint8_t  rx_size
                  );
