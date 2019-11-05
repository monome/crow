#pragma once

#include <stm32f7xx.h>
#include "../lib/ii.h" // I2C_RxCpltCallback()
#include "interrupts.h" // I2C_Priority

//////////////////////////////////////////
// hardware configuration

#define I2C_TIMING  0x50333090 // based on 400kHz @48MHz i2c clock

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


///////////////////////////
// queue setup
#define I2C_MAX_CMD_BYTES 10
#define I2C_BUFFER_LEN    32


//////////////////////////////////////////////////////////
// define these functions in your application program
extern void I2C_Follow_RxCallback( uint8_t* data );
extern void I2C_Lead_RxCallback( uint8_t address, uint8_t cmd, uint8_t* data );


/////////////////
// setup
uint8_t I2C_Init( uint8_t address );
void I2C_DeInit( void );

uint8_t I2C_is_boot( void );

void I2C_SetPullups( uint8_t state );

uint8_t I2C_GetAddress( void );
void I2C_SetAddress( uint8_t address );


/////////////////////////
// lead the bus
int I2C_is_ready( void );
int I2C_LeadTx( uint8_t  address
              , uint8_t* data
              , uint8_t  size
              );
int I2C_LeadRx( uint8_t  address
              , uint8_t* data
              , uint8_t  size
              , uint8_t  rx_size
              );

void I2C_BufferRx( uint8_t* data );
uint8_t* I2C_PopFollowBuffer( void );
uint8_t I2C_FollowBufferNotEmpty( void );
uint8_t* I2C_PopLeadBuffer( void );
uint8_t I2C_LeadBufferNotEmpty( void );
void I2C_SetTxData( uint8_t* data, uint8_t size );

//////////////////////////////////////////
// public handlers for HAL
void I2Cx_EV_IRQHandler( void );

// leader
void HAL_I2C_MasterTxCpltCallback( I2C_HandleTypeDef* h );
void HAL_I2C_MasterRxCpltCallback( I2C_HandleTypeDef* h );

// follower
void HAL_I2C_AddrCallback( I2C_HandleTypeDef* h
                         , uint8_t            TransferDirection
                         , uint16_t           AddrMatchCode );
void HAL_I2C_SlaveRxCpltCallback( I2C_HandleTypeDef* h );
void HAL_I2C_SlaveTxCpltCallback( I2C_HandleTypeDef* h );
void HAL_I2C_ListenCpltCallback( I2C_HandleTypeDef* hi2c );

// error
void I2Cx_ER_IRQHandler( void );
void HAL_I2C_ErrorCallback( I2C_HandleTypeDef* h );
