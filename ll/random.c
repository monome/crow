#include "random.h"

#include <stm32f7xx.h>

RNG_HandleTypeDef r;

#define rrrMAX 16
float rrrFm[rrrMAX];
int8_t rrrNext = 0;
int8_t rrrCount = 0;

void Random_Init(void)
{
    // config r
    r.Instance = RNG;
    HAL_RNG_Init(&r);

    // TODO need a HAL_Delay(1) here?
    if( HAL_RNG_GenerateRandomNumber_IT(&r) ){
        printf("rng failed to start\n");
    }
}

void Random_DeInit(void)
{
    HAL_RNG_DeInit(&r);
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
    __RNG_CLK_ENABLE(); // __HAL_RCC_RNG_CLK_ENABLE()

    HAL_NVIC_SetPriority( RNG_IRQn
                        , RNG_IRQPriority
                        , RNG_IRQSubPriority);
    HAL_NVIC_EnableIRQ( RNG_IRQn );
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
    __RNG_FORCE_RESET();
    __RNG_RELEASE_RESET();
}

//FIXME what is this IRQHandler?
void RNG_IRQHandler( void )
{
    HAL_RNG_IRQHandler(&r);
}

// handles both clock and seed errors. see HAL docs for how to recover
void HAL_RNG_ErrorCallback( RNG_HandleTypeDef* hr )
{
    printf("rng error\n");
}

void HAL_RNG_ReadyDataCallback( RNG_HandleTypeDef* hr, uint32_t rand32 )
{
    if( rrrCount < rrrMAX ){ // make sure we have room to store the val
        int8_t ix = (rrrNext + rrrCount++) % rrrMAX;
        rrrFm[ix] = (float)rand32 / (1.0 + (float)(uint32_t)0xFFFFFFFF);
        Random_Update();
    }
}

float Random_Get(void)
{
    if( rrrCount <= 0 ){ printf("rng: underrun\n"); }
    float retval = rrrFm[rrrNext++];
    rrrNext %= rrrMAX;
    if( --rrrCount < (rrrMAX/4) ){ // update if 3/4 empty
        Random_Update();
    }
    return retval;
}

void Random_Update(void)
{
    if( rrrCount != rrrMAX ){
        if( HAL_RNG_GenerateRandomNumber_IT(&r) ){
            printf("rng failed to fill\n");
        }
    }
}
