#pragma once

#include <stm32f7xx.h>
#include <stdbool.h>

typedef enum{ I2C_HW_PULLUP_OFF = 0
            , I2C_HW_PULLUP_ON = 1
} I2C_HW_PULLUP_STATE;

void i2c_hw_pullups_init(void);

void i2c_hw_pullups(I2C_HW_PULLUP_STATE is_enabled);
I2C_HW_PULLUP_STATE i2c_get_hw_pullups(void);
