#pragma once

#include <stm32f7xx.h>

void Debug_Pin_Init( void );

void Debug_Pin_Set( uint8_t state );

// Hardware Abstraction Defines
#define DBG_P_RCC_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define DBG_P_PIN           GPIO_PIN_9
#define DBG_P_GPIO          GPIOB
