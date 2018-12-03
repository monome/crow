#include "timers.h"

#include <stm32f7xx.h>
#include <stdlib.h>

#include "interrupts.h"
#include "../usbd/usbd_cdc_interface.h" // USB_TIM_PeriodElapsedCallback()

#include "debug_usart.h"

#define MAX_LL_TIMERS 9 // tell caller how many timers can be allocated

typedef void (*TIM_CLK_ENABLE_t)();

typedef struct{
    TIM_TypeDef*  Instance;
    const IRQn_Type     IRQn;
    TIM_CLK_ENABLE_t    CLK_ENABLE;
} Timer_setup_t;

// function wrappers over HAL macros
// TODO add TIM1 & TIM8. use different HAL functions
//static void TIM1_CLK_EN(){  __HAL_RCC_TIM1_CLK_ENABLE  }
//static void TIM2_CLK_EN(){  __HAL_RCC_TIM2_CLK_ENABLE();  } // ADS
//static void TIM3_CLK_EN(){  __HAL_RCC_TIM3_CLK_ENABLE();  } // USBD
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
    { //{ TIM1  , TIM1_IRQn  , TIM1_CLK_EN  }
    //, { TIM2  , TIM2_IRQn               , TIM2_CLK_EN  }
    //, { TIM3  , TIM3_IRQn               , TIM3_CLK_EN  }
     { TIM4  , TIM4_IRQn               , TIM4_CLK_EN  }
    , { TIM5  , TIM5_IRQn               , TIM5_CLK_EN  }
    , { TIM6  , TIM6_DAC_IRQn           , TIM6_CLK_EN  }
    , { TIM7  , TIM7_IRQn               , TIM7_CLK_EN  }
    //, { TIM8  , TIM8_IRQn  , TIM8_CLK_EN  }
    , { TIM9  , TIM1_BRK_TIM9_IRQn      , TIM9_CLK_EN  }
    , { TIM10 , TIM1_UP_TIM10_IRQn      , TIM10_CLK_EN }
    , { TIM11 , TIM1_TRG_COM_TIM11_IRQn , TIM11_CLK_EN }
    , { TIM12 , TIM8_BRK_TIM12_IRQn     , TIM12_CLK_EN }
    , { TIM13 , TIM8_UP_TIM13_IRQn      , TIM13_CLK_EN }
    , { TIM14 , TIM8_TRG_COM_TIM14_IRQn , TIM14_CLK_EN }
    };

TIM_HandleTypeDef TimHandle[MAX_LL_TIMERS];
Timer_Callback_t callback;

// FIXME have to manually index the following
//void TIM1_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[0] ); }
//void TIM2_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[0] ); }
//void TIM3_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[0] ); }
void TIM4_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[0] ); }
void TIM5_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[1] ); }
void TIM6_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[2] ); }
void TIM7_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[3] ); }
//void TIM8_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[0] ); }
void TIM9_IRQHandler( void ){  HAL_TIM_IRQHandler( &TimHandle[4] ); }
void TIM10_IRQHandler( void ){ HAL_TIM_IRQHandler( &TimHandle[5] ); }
void TIM11_IRQHandler( void ){ HAL_TIM_IRQHandler( &TimHandle[6] ); }
void TIM12_IRQHandler( void ){ HAL_TIM_IRQHandler( &TimHandle[7] ); }
void TIM13_IRQHandler( void ){ HAL_TIM_IRQHandler( &TimHandle[8] ); }

int Timer_Init( Timer_Callback_t cb )
{
    U_PrintLn("Timer_Init");
    callback = cb;
    for( int i=0; i<MAX_LL_TIMERS; i++ ){
        TimHandle[i].Instance = _timer[i].Instance;

        // static setup
        TimHandle[i].Init.ClockDivision     = TIM_CLOCKDIVISION_DIV4;
        TimHandle[i].Init.CounterMode       = TIM_COUNTERMODE_UP;
        TimHandle[i].Init.RepetitionCounter = 0;
        TimHandle[i].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        Timer_Set_Params( i, 1.0, -1 );
    }
    //Timer_Start(0);

    return MAX_LL_TIMERS;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    for( int i=0; i<MAX_LL_TIMERS; i++ ){
        if( htim == &TimHandle[i] ){
// FIXME how to do this? this is a macro not fn :/
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
    // only iterate if it *doesn't* match the USB handle
    if( !USB_TIM_PeriodElapsedCallback(htim) ){
        for( int i=0; i<MAX_LL_TIMERS; i++ ){
            if( htim == &TimHandle[i] ){
                // TODO countdown the repeat counter (is this hardware?)
                (*callback)(i); // raise callback
                return;
            }
        }
    }
}

void Timer_Set_Params( int ix, float seconds, int count )
{
    //TODO need to call Timer_Stop(ix); first to deactivate while changing params?
    //FIXME limited to max~16s. possible to decrease speed of clock source?


    TimHandle[ix].Init.Period            = (uint16_t)(seconds * (float)0x1000);
    TimHandle[ix].Init.Prescaler         = 0xE000; // TODO
    TimHandle[ix].Init.RepetitionCounter = 0; // TODO is this a counter?? seems not
    if( HAL_TIM_Base_Init( &TimHandle[ix] ) != HAL_OK ){
        U_Print("Timer_Set_Params("); U_PrintU8n(ix); U_PrintLn(") failed");
    }
}

void Timer_Start( int ix )
{
    if( HAL_TIM_Base_Start_IT( &TimHandle[ix] ) != HAL_OK ){
        U_Print("Timer_Start("); U_PrintU8n(ix); U_PrintLn(") failed");
    }
}

void Timer_Stop( int ix )
{
    if( HAL_TIM_Base_Stop_IT( &TimHandle[ix] ) != HAL_OK ){
        U_Print("Timer_Stop("); U_PrintU8n(ix); U_PrintLn(") failed");
    }
}
