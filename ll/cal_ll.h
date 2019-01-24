#pragma once

#include <stm32f7xx.h>

// Public Interface
typedef enum { CAL_LL_dac3
             , CAL_LL_dac2
             , CAL_LL_dac1
             , CAL_LL_dac0
             , CAL_LL_2v5
             , CAL_LL_Ground
             , CAL_LL_ChannelList
} CAL_LL_Channel_t;

void CAL_LL_Init( void );
void CAL_LL_ActiveChannel( CAL_LL_Channel_t channel );

// Hardware Defines
#define CAL_0_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define CAL_1_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define CAL_2_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()

#define CAL_0_PIN           GPIO_PIN_7
#define CAL_0_GPIO_PORT     GPIOC
#define CAL_1_PIN           GPIO_PIN_9
#define CAL_1_GPIO_PORT     GPIOC
#define CAL_2_PIN           GPIO_PIN_8
#define CAL_2_GPIO_PORT     GPIOC
