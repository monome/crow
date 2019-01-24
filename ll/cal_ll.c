#include "cal_ll.h"

// Public Definitions
void CAL_LL_Init( void )
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    CAL_0_GPIO_CLK_ENABLE();
    CAL_1_GPIO_CLK_ENABLE();
    CAL_2_GPIO_CLK_ENABLE();

    // GPIO pins
    GPIO_InitStruct.Pull   = GPIO_PULLUP;
    GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed  = GPIO_SPEED_LOW;

    GPIO_InitStruct.Pin    = CAL_0_PIN;
    HAL_GPIO_Init( CAL_0_GPIO_PORT, &GPIO_InitStruct );
    GPIO_InitStruct.Pin    = CAL_1_PIN;
    HAL_GPIO_Init( CAL_1_GPIO_PORT, &GPIO_InitStruct );
    GPIO_InitStruct.Pin    = CAL_2_PIN;
    HAL_GPIO_Init( CAL_2_GPIO_PORT, &GPIO_InitStruct );

    CAL_LL_ActiveChannel( CAL_LL_Ground );
}
void CAL_LL_ActiveChannel( CAL_LL_Channel_t channel )
{
    if( channel >= CAL_LL_ChannelList ){ return; }
    HAL_GPIO_WritePin( CAL_0_GPIO_PORT
                     , CAL_0_PIN
                     , (channel & 1)
                     );
    HAL_GPIO_WritePin( CAL_1_GPIO_PORT
                     , CAL_1_PIN
                     , (channel & 2) >> 1
                     );
    HAL_GPIO_WritePin( CAL_2_GPIO_PORT
                     , CAL_2_PIN
                     , (channel & 4) >> 2
                     );
}
