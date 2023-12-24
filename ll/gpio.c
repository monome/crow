#include "gpio.h"

#include <stm32f7xx.h>
#include <stdio.h>

// TODO we can log every portpin and check for conflicts
void gpio_config( Gpio* g, Gpio_init init )
{
    // Convert ascii port name to GPIO_TypeDef & init RCC clock
    switch(init.portpin[0]){
        case 'a': case 'A': __HAL_RCC_GPIOA_CLK_ENABLE(); g->port = GPIOA; break;
        case 'b': case 'B': __HAL_RCC_GPIOB_CLK_ENABLE(); g->port = GPIOB; break;
        case 'c': case 'C': __HAL_RCC_GPIOC_CLK_ENABLE(); g->port = GPIOC; break;
        case 'd': case 'D': __HAL_RCC_GPIOD_CLK_ENABLE(); g->port = GPIOD; break;
        case 'e': case 'E': __HAL_RCC_GPIOE_CLK_ENABLE(); g->port = GPIOE; break;
        case 'f': case 'F': __HAL_RCC_GPIOF_CLK_ENABLE(); g->port = GPIOF; break;
        case 'g': case 'G': __HAL_RCC_GPIOG_CLK_ENABLE(); g->port = GPIOG; break;
        default: printf("gpio port not found\n"); return;
    }

    // Convert ascii -> pin_number -> pin mask
    g->pin = 1 << gpio_portpin_to_pin( init.portpin ); // convert index to mask

    gpio_reconfig( g, init );
}

uint8_t gpio_portpin_to_pin( char* portpin )
{
    // ascii 48 == numeral 0
    uint8_t pin;
    if( portpin[2] < 48 ){ // assume last character is null (so single digit pin)
        pin = (uint8_t)portpin[1] - 48;
    } else { // assume 2 digits
        pin = 10 * ((uint8_t)portpin[1] - 48);
        pin += (uint8_t)portpin[2] - 48;
    }
    return pin;
}

// change configuration of portpin (can't change pin assignment)
void gpio_reconfig( Gpio* g, Gpio_init init )
{
    // TODO Add defaults to optional args (pull/speed/alternate)
    GPIO_InitTypeDef ginit = { .Pin       = g->pin
                             , .Mode      = (uint32_t)init.mode
                             , .Pull      = (uint32_t)init.pull
                             , .Speed     = (uint32_t)init.speed
                             , .Alternate = (uint32_t)init.af
                             };
    HAL_GPIO_Init( g->port, &ginit );
}

void gpio_reconfig_easy( Gpio* g, Gpio_type t )
{
    switch(t){
        case Gpio_type_Din:
            gpio_reconfig( g,
                (Gpio_init){ .mode = GPIO_MODE_INPUT
                           , .pull = GPIO_NOPULL
                           });
            break;
        case Gpio_type_Dout:
            gpio_reconfig( g,
                (Gpio_init){ .mode = GPIO_MODE_OUTPUT_PP
                           , .pull = GPIO_NOPULL
                           });
            break;
    }
}
