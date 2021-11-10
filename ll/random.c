#include "random.h"

#include <stm32f7xx.h>
#include <stdio.h>

static RNG_HandleTypeDef r;

#define BUFLEN 16
static float buf[BUFLEN];
static int8_t head = 0;
static int8_t count = 0;

void Random_Init(void)
{
    r.Instance = RNG;
    HAL_RNG_Init(&r);

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

// Forward LL callback to HAL
void RNG_IRQHandler( void )
{
    HAL_RNG_IRQHandler(&r);
}

// handles both clock and seed errors. see HAL docs for how to recover
void HAL_RNG_ErrorCallback( RNG_HandleTypeDef* hr )
{
    printf("rng error\n");
    // Just reset the RNG and hope for the best!
    // should handle seed errors correctly, and is overkill for clock error
    Random_DeInit(); // clears SEIS bit (recover from seed error)
    Random_Init();
}

void HAL_RNG_ReadyDataCallback( RNG_HandleTypeDef* hr, uint32_t rand32 )
{
    if( count < BUFLEN ){ // make sure we have room to store the val
        int8_t ix = (head + count++) % BUFLEN;
        buf[ix] = (float)rand32 / (1.0 + (float)(uint32_t)0xFFFFFFFF);
        Random_Update();
    }
}

float Random_Float(void)
{
    if( count <= 0 ){ printf("rng: underrun\n"); }
    float retval = buf[head++];
    head %= BUFLEN;
    if( --count < (BUFLEN/4) ){ // update if 3/4 empty
        Random_Update();
    }
    return retval;
}

int Random_Int(int lower, int upper)
{
    return (int)(Random_Float() * (float)(1 + upper - lower)) + lower;
}

void Random_Update(void)
{
    if( count != BUFLEN ){
        if( HAL_RNG_GenerateRandomNumber_IT(&r) ){
            printf("rng failed to fill\n");
        }
    }
}
