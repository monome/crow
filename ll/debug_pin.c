#include "debug_pin.h"

GPIO_InitTypeDef GPIO_InitStruct;
void Debug_Pin_Init(void)
{
    DBG_P_RCC_ENABLE();

    GPIO_InitStruct.Pin   = DBG_P_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(DBG_P_GPIO, &GPIO_InitStruct);
}

void Debug_Pin_Set( uint8_t state )
{
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
    HAL_GPIO_WritePin( DBG_P_GPIO, DBG_P_PIN, state );
__set_PRIMASK( old_primask );
}
