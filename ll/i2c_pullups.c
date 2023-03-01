#include "i2c_pullups.h"

static I2C_HW_PULLUP_STATE pullup_state = I2C_HW_PULLUP_OFF;
static GPIO_InitTypeDef GPIO_IS;

// GPIO: B3 & B4

void i2c_hw_pullups_init(void){
    // activate peripheral clock
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // set constant params of gpio init struct
    GPIO_IS.Pin   = GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_IS.Pull  = GPIO_NOPULL;
    GPIO_IS.Speed = GPIO_SPEED_LOW;

    // default to pullups ON
    i2c_hw_pullups(I2C_HW_PULLUP_ON);
}

void i2c_hw_pullups(I2C_HW_PULLUP_STATE is_enabled){
    pullup_state = is_enabled; // save state for inspection
    switch(is_enabled){
        case I2C_HW_PULLUP_OFF:
            // deactivate output drive
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

            // set pins to high-impedance
            GPIO_IS.Mode = GPIO_MODE_INPUT;
            HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);
            HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4);
            HAL_GPIO_Init(GPIOB, &GPIO_IS);
            break;

        case I2C_HW_PULLUP_ON:
            // set pins to output
            GPIO_IS.Mode = GPIO_MODE_OUTPUT_PP;
            HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);
            HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4);
            HAL_GPIO_Init(GPIOB, &GPIO_IS);
            
            // activate output drive
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
            break;
    }
}

I2C_HW_PULLUP_STATE i2c_get_hw_pullups(void){
    return pullup_state;
}
