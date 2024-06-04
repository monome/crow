#include "debug_pin.h"

GPIO_InitTypeDef GPIO_InitStruct;
void Debug_Pin_Init(void)
{
    DBG_P_RCC_ENABLE();

    GPIO_InitStruct.Pin   = DBG_P_PIN_B | DBG_P_PIN_R | DBG_P_PIN_G;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(DBG_P_GPIO, &GPIO_InitStruct);
}

void Debug_Pin_Set(int chan, int state ){
    switch(chan){
        case 0: HAL_GPIO_WritePin( DBG_P_GPIO, DBG_P_PIN_R, state ); break;
        case 1: HAL_GPIO_WritePin( DBG_P_GPIO, DBG_P_PIN_G, state ); break;
        case 2: HAL_GPIO_WritePin( DBG_P_GPIO, DBG_P_PIN_B, state ); break;
    }
}

void Debug_Error_state(void){
    Debug_Pin_Set(0, 0);
    Debug_Pin_Set(1, 0);
    Debug_Pin_Set(2, 1);
}
