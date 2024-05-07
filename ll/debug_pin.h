#pragma once

#include <stm32f7xx.h>

void Debug_Pin_Init( void );

// void Debug_Pin_Set( uint8_t state );
void Debug_Pin_Set(int chan, int state );

// Hardware Abstraction Defines
// this is the top most LED to the right of the uC
#define DBG_P_RCC_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()
#define DBG_P_GPIO          GPIOE
#define DBG_P_PIN_B         GPIO_PIN_13
#define DBG_P_PIN_R         GPIO_PIN_14
#define DBG_P_PIN_G         GPIO_PIN_15
