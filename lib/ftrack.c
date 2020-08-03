#include "ftrack.h"

// time constant
#define FTRACK_SMOOTHING 0.1 // ~60 samples of settling time (freq dependent)
//#define FTRACK_SLEW      0.013 // 500 blocks == 333ms
#define FTRACK_SLEW      0.013 // 500 blocks == 333ms
    // reducing this value does not increase accuracy substantially 

// globals
static int blocks, zc;
static float last;
static float z;
static float zz;


///////////////////////////////////
// Init

void FTrack_init( void )
{
    FTRACK_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef g = { .Pin  = FTRACK_PIN
                         , .Mode = GPIO_MODE_IT_RISING
                         , .Pull = GPIO_PULLUP
                         };
    // TODO confirm what happens when alternating with MIDI (uses same pin)
    HAL_GPIO_Init( FTRACK_GPIO_PORT, &g );
}

void FTrack_deinit( void )
{
    // TODO
}


////////////////////////////////
// Configuration

void FTrack_start( void )
{
    // reset parameters
    blocks = 0;
    zc     = 0;
    last   = 0.0;
    z      = 0.0;
    zz     = 0.0;
    HAL_NVIC_SetPriority( FTRACK_IRQn
                        , FTRACK_IRQPriority
                        , FTRACK_IRQSubPriority
                        );
    HAL_NVIC_EnableIRQ( FTRACK_IRQn );
}

void FTrack_stop( void )
{
    HAL_NVIC_DisableIRQ( FTRACK_IRQn );
}

float FTrack_get( float frequency )
{
    blocks++;
    if( zc > 0 ){ // new zero-crossings this frame
        // apply LP1 relative to input freq rate
        float dest = frequency * (float)zc / (float)blocks;
        last = last + FTRACK_SMOOTHING * (dest-last);

        // reset counters
        blocks = 0;
        zc     = 0;
    }
    // signal rate slews. 2xLP1
    z  = z  + FTRACK_SLEW * (last- z );
    zz = zz + FTRACK_SLEW * (z   - zz);
    return zz;
}


////////////////////////////////
// Interrupt handler

void FTRACK_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler( FTRACK_PIN );
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if( GPIO_Pin == FTRACK_PIN ){
        zc++;
    }
}
