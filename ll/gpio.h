#pragma once

#include <stdint.h>

#include <stm32f7xx.h> // GPIO_TypeDef

// helper library for using string names for port assignments

typedef struct{
    // config
    char*    portpin; // eg A0 or I13

    // essential config
    uint32_t mode;

    // optional config
    uint32_t speed;
    uint32_t pull;
    uint32_t af;
} Gpio_init;

typedef enum{ Gpio_type_Dout
            , Gpio_type_Din
} Gpio_type;

typedef struct{
    GPIO_TypeDef* port;
    uint16_t      pin;
} Gpio;


void gpio_config( Gpio* g, Gpio_init init ); // initialize & configure portpin
uint8_t gpio_portpin_to_pin( char* portpin ); // convert portpin string to pin index (not mask)
void gpio_reconfig( Gpio* g, Gpio_init init ); // reconfigure an already initialized portpin (mode/pull/af)
void gpio_reconfig_easy( Gpio* g, Gpio_type t ); // reconfigure with init templates
