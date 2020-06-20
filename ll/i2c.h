#pragma once

#include <stm32f7xx.h>
#include <stdbool.h>
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


///////////////////////////////////////
// fnptr typedefs for low-level
typedef void (*I2C_lead_callback_t)( uint8_t address, uint8_t command, uint8_t* rx_data );
typedef int (*I2C_follow_callback_t)( uint8_t* pdata );
typedef void (*I2C_error_callback_t)( int error_code );


/////////////////
// setup
uint8_t I2C_Init( uint8_t               address
                , I2C_lead_callback_t   lead_callback
                , I2C_follow_callback_t follow_action_callback
                , I2C_follow_callback_t follow_request_callback
                , I2C_error_callback_t  error_callback );
void I2C_DeInit( void );

uint8_t I2C_is_boot( void );

void I2C_SetPullups( uint8_t state );
uint8_t I2C_GetPullups( void );

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
