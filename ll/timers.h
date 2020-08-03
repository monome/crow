#pragma once

// backend to the lib/metro.c functions

// TIMER
//
//
// nb: TIM2 is used by the ADS131 driver but all others are available
//
//  /*  advanced control timers */
// TIM1
// TIM8
//
// /*  general-purpose timer */
// TIM2 // used by ADS131
// TIM3
// TIM4
// TIM5
//
// /*  basic timers */
// TIM6
// TIM7
//
// /*  general purpose timers 2 */
// TIM9
// TIM10
// TIM11
// TIM12
// TIM13
// TIM14
//
//
//#define TIMx                    TIM3
//#define TIMx_CLK_ENABLE()       __HAL_RCC_TIM3_CLK_ENABLE()
//
//#define TIMx_IRQn               TIM3_IRQn
//#define TIMx_IRQHandler         TIM3_IRQHandler
#define TIMx_IRQ_Priority       TIM_IRQPriority
#define TIMx_IRQ_SubPriority    0

typedef void (*Timer_Callback_t)(int);

int Timer_Init(void);

void Timer_Start( int ix, Timer_Callback_t cb );
void Timer_Stop( int ix );
void Timer_Set_Params( int ix, float seconds );
void Timer_Priority( int ix, int priority_level );
