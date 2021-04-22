#include "timers.h"

#include <stm32f7xx.h>
#include <stdlib.h>
#include <stdio.h>

#include "interrupts.h"

#define MAX_LL_TIMERS 11 // tell caller how many timers can be allocated

typedef void (*TIM_CLK_ENABLE_t)();

typedef struct{
    TIM_TypeDef*        Instance;
    const IRQn_Type     IRQn;
    TIM_CLK_ENABLE_t    CLK_ENABLE;
} Timer_setup_t;

// function wrappers over HAL macros
// TODO add TIM1 & TIM8. use different HAL functions
//static void TIM1_CLK_EN(){  __HAL_RCC_TIM1_CLK_ENABLE  }
//static void TIM2_CLK_EN(){  __HAL_RCC_TIM2_CLK_ENABLE();  } // ADS
static void TIM3_CLK_EN(){  __HAL_RCC_TIM3_CLK_ENABLE();  }
static void TIM4_CLK_EN(){  __HAL_RCC_TIM4_CLK_ENABLE();  }
static void TIM5_CLK_EN(){  __HAL_RCC_TIM5_CLK_ENABLE();  }
static void TIM6_CLK_EN(){  __HAL_RCC_TIM6_CLK_ENABLE();  }
static void TIM7_CLK_EN(){  __HAL_RCC_TIM7_CLK_ENABLE();  }
//static void TIM8_CLK_EN(){  __HAL_RCC_TIM8_CLK_ENABLE();  }
static void TIM9_CLK_EN(){  __HAL_RCC_TIM9_CLK_ENABLE();  }
static void TIM10_CLK_EN(){ __HAL_RCC_TIM10_CLK_ENABLE(); }
static void TIM11_CLK_EN(){ __HAL_RCC_TIM11_CLK_ENABLE(); }
static void TIM12_CLK_EN(){ __HAL_RCC_TIM12_CLK_ENABLE(); }
static void TIM13_CLK_EN(){ __HAL_RCC_TIM13_CLK_ENABLE(); }
static void TIM14_CLK_EN(){ __HAL_RCC_TIM14_CLK_ENABLE(); }

const Timer_setup_t _timer[]=
    { //{ TIM1  , TIM1_IRQn               , TIM1_CLK_EN  }
    //, { TIM2  , TIM2_IRQn               , TIM2_CLK_EN  }
      { TIM3  , TIM3_IRQn               , TIM3_CLK_EN  }
    , { TIM4  , TIM4_IRQn               , TIM4_CLK_EN  }
    , { TIM5  , TIM5_IRQn               , TIM5_CLK_EN  }
    , { TIM6  , TIM6_DAC_IRQn           , TIM6_CLK_EN  }
    , { TIM7  , TIM7_IRQn               , TIM7_CLK_EN  } // seems to work 0x37/d55
    //, { TIM8  , TIM8_IRQn               , TIM8_CLK_EN  }
    , { TIM9  , TIM1_BRK_TIM9_IRQn      , TIM9_CLK_EN  }
    , { TIM10 , TIM1_UP_TIM10_IRQn      , TIM10_CLK_EN }
    , { TIM11 , TIM1_TRG_COM_TIM11_IRQn , TIM11_CLK_EN }
    , { TIM12 , TIM8_BRK_TIM12_IRQn     , TIM12_CLK_EN }
    , { TIM13 , TIM8_UP_TIM13_IRQn      , TIM13_CLK_EN }
    , { TIM14 , TIM8_TRG_COM_TIM14_IRQn , TIM14_CLK_EN }
    };

TIM_HandleTypeDef TimHandle[MAX_LL_TIMERS];
Timer_Callback_t callback[MAX_LL_TIMERS];

// FIXME have to manually index the following
void TIM3_IRQHandler(               void ){ HAL_TIM_IRQHandler( &(TimHandle[0]) ); }
void TIM4_IRQHandler(               void ){ HAL_TIM_IRQHandler( &(TimHandle[1]) ); }
void TIM5_IRQHandler(               void ){ HAL_TIM_IRQHandler( &(TimHandle[2]) ); }
void TIM6_DAC_IRQHandler(           void ){ HAL_TIM_IRQHandler( &(TimHandle[3]) ); }
void TIM7_IRQHandler(               void ){ HAL_TIM_IRQHandler( &(TimHandle[4]) ); }
void TIM1_BRK_TIM9_IRQHandler(      void ){ HAL_TIM_IRQHandler( &(TimHandle[5]) ); }
void TIM1_UP_TIM10_IRQHandler(      void ){ HAL_TIM_IRQHandler( &(TimHandle[6]) ); }
void TIM1_TRG_COM_TIM11_IRQHandler( void ){ HAL_TIM_IRQHandler( &(TimHandle[7]) ); }
void TIM8_BRK_TIM12_IRQHandler(     void ){ HAL_TIM_IRQHandler( &(TimHandle[8]) ); }
void TIM8_UP_TIM13_IRQHandler(      void ){ HAL_TIM_IRQHandler( &(TimHandle[9]) ); }
void TIM8_TRG_COM_TIM14_IRQHandler( void ){ HAL_TIM_IRQHandler( &(TimHandle[10]) ); }

int Timer_Init(void)
{
    for( int i=0; i<MAX_LL_TIMERS; i++ ){
        TimHandle[i].Instance = _timer[i].Instance;

        // static setup
        TimHandle[i].Init.ClockDivision     = TIM_CLOCKDIVISION_DIV4;

        TimHandle[i].Init.CounterMode       = TIM_COUNTERMODE_UP;
        TimHandle[i].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        Timer_Set_Params( i, 1.0 );
    }
    return MAX_LL_TIMERS;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    for( int i=0; i<MAX_LL_TIMERS; i++ ){
        if( htim == &(TimHandle[i]) ){
            (_timer[i].CLK_ENABLE)();
            HAL_NVIC_SetPriority( _timer[i].IRQn
                                , TIMx_IRQ_Priority // global timer priority
                                , 0                 // same subpriority
                                );
            HAL_NVIC_EnableIRQ( _timer[i].IRQn );
        }
    }
}

void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef *htim )
{
    // counting down to prioritize statically allocated timers (eg usb)
    for( int i=(MAX_LL_TIMERS-1); i>=0; i-- ){
        if( htim == &(TimHandle[i]) ){
            (*callback[i])(i);
            return;
        }
    }
}

void Timer_Set_Params( int ix, float seconds )
{
    //FIXME limited to max~20s (p=0xFFFF & ps=0xFFFF)
    const double SECOND_SCALER = ((double)216000000.0 / (double)0x10000)-(double)1.0;

// do this multiplication in *double* precision bc seconds could be very small (uS), and SCALER is very big (3500)
    float pf = (double)seconds * SECOND_SCALER; // exact when ps == 0xFFFF
    uint16_t ps = 0xFFFF; // longest possible
    while( pf < (float)(0x7FFF) ){ // decrement prescaler to maximize accuracy
        ps >>= 1; // half the prescaler
        pf *= 2.0; // double period
        if( ps == 0 ){ break; }
    }
    uint16_t p = (pf < 0.0) ? 0x0
                    : (pf > (float)0xFFFF) ? 0xFFFF
                    : (uint16_t)pf;

    TimHandle[ix].Init.Period    = p;
    TimHandle[ix].Init.Prescaler = ps;
    uint8_t err;
    BLOCK_IRQS(
        err = HAL_TIM_Base_Init( &(TimHandle[ix]) );
        // Clear the update flag to avoid an instant callback
        __HAL_TIM_CLEAR_FLAG( &(TimHandle[ix]), TIM_FLAG_UPDATE );
    );
    if( err != HAL_OK ){
        printf("Timer_Set_Params(%i) failed\n", ix);
    }
}

void Timer_Start( int ix, Timer_Callback_t cb )
{
    uint8_t err;
    callback[ix] = cb;
    BLOCK_IRQS(
        err = HAL_TIM_Base_Start_IT( &(TimHandle[ix]) );
    );
    if( err != HAL_OK ){
        printf("Timer_Start(%i) failed\n", ix);
    }
}

void Timer_Stop( int ix )
{
    uint8_t err;
    BLOCK_IRQS(
        err = HAL_TIM_Base_Stop_IT( &(TimHandle[ix]) );
    );
    if( err != HAL_OK ){
        printf("Timer_Stop(%i)\n", ix);
    }
}

void Timer_Priority( int ix, int priority_level )
{
    HAL_NVIC_DisableIRQ( _timer[ix].IRQn );
    HAL_NVIC_SetPriority( _timer[ix].IRQn
                        , priority_level
                        , 0
                        );
    HAL_NVIC_EnableIRQ( _timer[ix].IRQn );
}

