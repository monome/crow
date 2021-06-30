#pragma once

#include <stm32f7xx.h>
#include "../ll/interrupts.h"

// shared pins with MIDI port
#define FTRACK_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define FTRACK_PIN               GPIO_PIN_1
#define FTRACK_GPIO_PORT         GPIOA

#define FTRACK_IRQn                  EXTI1_IRQn
#define FTRACK_IRQHandler            EXTI1_IRQHandler
#define FTRACK_IRQPriority           MIDI_IRQPriority
#define FTRACK_IRQSubPriority        2


void FTrack_init( void );
void FTrack_deinit( void );

void FTrack_start( void );
void FTrack_stop( void );

float FTrack_get( void );

// Interrupt handler for HAL
void FTRACK_IRQHandler(void);
