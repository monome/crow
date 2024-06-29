#include "adc.h"

#include <stdio.h>
// #include <stm32f7xx_hal.h> // HAL_Delay()

#include "interrupts.h"
#include "../lib/caw.h"

static ADC_HandleTypeDef AdcHandle;
static ADC_HandleTypeDef AdcHandle2;
static ADC_ChannelConfTypeDef sConfig;

#define ADC_CHANNELS 3
#define ADC_BUFFERS 2
static volatile uint16_t adc_raw[ADC_CHANNELS*ADC_BUFFERS*2]; // 6 channels,
static volatile uint16_t* adc_raw2 = &adc_raw[ADC_CHANNELS*ADC_BUFFERS*1]; // second 3 channels


static void start_conversion(void);

void ADC_Init(void){
    AdcHandle.Instance                   = ADC1;
    HAL_ADC_DeInit(&AdcHandle);

    AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
    AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
    AdcHandle.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    AdcHandle.Init.ContinuousConvMode    = ENABLE; // i think disable
    AdcHandle.Init.NbrOfConversion       = 3; // scan all 5 chans on ADC1
    AdcHandle.Init.DiscontinuousConvMode = DISABLE; // i think enable
    AdcHandle.Init.NbrOfDiscConversion   = 3;
    AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
    AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    AdcHandle.Init.DMAContinuousRequests = ENABLE;
    AdcHandle.Init.EOCSelection          = ADC_EOC_SEQ_CONV;

    if( HAL_ADC_Init( &AdcHandle ) != HAL_OK ){
        Caw_printf("HAL_ADC_Init failed\n");
    }

    // Channel configuration
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    // sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    sConfig.Offset       = 0;

    sConfig.Channel      = ADC_CHANNEL_7;
    sConfig.Rank         = 1;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        Caw_printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_8;
    sConfig.Rank         = 2;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        Caw_printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_9;
    sConfig.Rank         = 3;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        Caw_printf("HAL_ADC_ConfigChannel failed\n");



    AdcHandle2.Instance                   = ADC2;
    HAL_ADC_DeInit(&AdcHandle2);

    AdcHandle2.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
    AdcHandle2.Init.Resolution            = ADC_RESOLUTION_12B;
    AdcHandle2.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    AdcHandle2.Init.ContinuousConvMode    = ENABLE; // i think disable
    AdcHandle2.Init.NbrOfConversion       = 3; // scan all 5 chans on ADC1
    AdcHandle2.Init.DiscontinuousConvMode = DISABLE; // i think enable
    AdcHandle2.Init.NbrOfDiscConversion   = 3;
    AdcHandle2.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    AdcHandle2.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    AdcHandle2.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
    AdcHandle2.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    AdcHandle2.Init.DMAContinuousRequests = ENABLE;
    AdcHandle2.Init.EOCSelection          = ADC_EOC_SEQ_CONV;

    if( HAL_ADC_Init( &AdcHandle2 ) != HAL_OK ){
        Caw_printf("HAL_ADC_Init2 failed\n");
    }

    sConfig.Channel      = ADC_CHANNEL_14;
    sConfig.Rank         = 1;
    if( HAL_ADC_ConfigChannel(&AdcHandle2, &sConfig) != HAL_OK )
        Caw_printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_5;
    sConfig.Rank         = 2;
    if( HAL_ADC_ConfigChannel(&AdcHandle2, &sConfig) != HAL_OK )
        Caw_printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_6;
    sConfig.Rank         = 3;
    if( HAL_ADC_ConfigChannel(&AdcHandle2, &sConfig) != HAL_OK )
        Caw_printf("HAL_ADC_ConfigChannel failed\n");


    start_conversion();
}

static void start_conversion(void){
    if( HAL_ADC_Start_DMA( &AdcHandle
                         , (uint32_t*)adc_raw
                         , ADC_CHANNELS*ADC_BUFFERS
                         ) != HAL_OK ){
        Caw_printf("HAL_ADC_Start_DMA failed, retrying..\n");
        // HAL_Delay(10);
        if( HAL_ADC_Start_DMA( &AdcHandle
                             , (uint32_t*)adc_raw
                             , ADC_CHANNELS*ADC_BUFFERS
                             ) != HAL_OK ){
            Caw_printf("HAL_ADC_Start_DMA failed again, ignoring\n");
            return;
        }
    }

    if( HAL_ADC_Start_DMA( &AdcHandle2
                         , (uint32_t*)adc_raw2
                         , ADC_CHANNELS*ADC_BUFFERS
                         ) != HAL_OK ){
        Caw_printf("HAL_ADC_Start_DMA failed, retrying..\n");
        // HAL_Delay(10);
        if( HAL_ADC_Start_DMA( &AdcHandle2
                             , (uint32_t*)adc_raw2
                             , ADC_CHANNELS*ADC_BUFFERS
                             ) != HAL_OK ){
            Caw_printf("HAL_ADC_Start_DMA failed again, ignoring\n");
            return;
        }
    }
    // Caw_printf("conversion started\n\r");
}

static DMA_HandleTypeDef  hdma_adc;
static DMA_HandleTypeDef  hdma_adc2;
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc){
    GPIO_InitTypeDef          gpio;

    if(hadc == &AdcHandle){ // ADC1
        __HAL_RCC_ADC1_CLK_ENABLE(); // just using ADC1 for now
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();
        gpio.Mode = GPIO_MODE_ANALOG;
        gpio.Pull = GPIO_NOPULL;

        // ADC1,2,3
        gpio.Pin  = GPIO_PIN_7;
        HAL_GPIO_Init(GPIOA, &gpio);
        gpio.Pin  = GPIO_PIN_0 | GPIO_PIN_1;
        HAL_GPIO_Init(GPIOB, &gpio);

        // ADC1: d2.s0.c0, d2.s4.c0
        hdma_adc.Instance = DMA2_Stream4; // or DMA2_stream0

        hdma_adc.Init.Channel               = DMA_CHANNEL_0;
        hdma_adc.Init.Direction             = DMA_PERIPH_TO_MEMORY;
        hdma_adc.Init.PeriphInc             = DMA_PINC_DISABLE;
        hdma_adc.Init.MemInc                = DMA_MINC_ENABLE;
        hdma_adc.Init.PeriphDataAlignment   = DMA_PDATAALIGN_HALFWORD;
        hdma_adc.Init.MemDataAlignment      = DMA_MDATAALIGN_WORD;
        hdma_adc.Init.Mode                  = DMA_CIRCULAR;
        hdma_adc.Init.Priority              = DMA_PRIORITY_HIGH;
        hdma_adc.Init.FIFOMode              = DMA_FIFOMODE_DISABLE;
        hdma_adc.Init.FIFOThreshold         = DMA_FIFO_THRESHOLD_HALFFULL;
        hdma_adc.Init.MemBurst              = DMA_MBURST_SINGLE;
        hdma_adc.Init.PeriphBurst           = DMA_PBURST_SINGLE;

        HAL_DMA_DeInit(&hdma_adc);
        HAL_DMA_Init(&hdma_adc);

        __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

        HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, ADC_IRQPriority, 1);
        HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

    } else if(hadc == &AdcHandle2){
        __HAL_RCC_ADC2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE();
        gpio.Mode = GPIO_MODE_ANALOG;
        gpio.Pull = GPIO_NOPULL;

        // ADC4,5,6
        gpio.Pin  = GPIO_PIN_4;
        HAL_GPIO_Init(GPIOC, &gpio);
        gpio.Pin  = GPIO_PIN_5 | GPIO_PIN_6;
        HAL_GPIO_Init(GPIOA, &gpio);

        // ADC2: d2.s2.c1, d2.s3.c1
        hdma_adc2.Instance = DMA2_Stream2; // Stream2/3, Channel1

        hdma_adc2.Init.Channel               = DMA_CHANNEL_1;
        hdma_adc2.Init.Direction             = DMA_PERIPH_TO_MEMORY;
        hdma_adc2.Init.PeriphInc             = DMA_PINC_DISABLE;
        hdma_adc2.Init.MemInc                = DMA_MINC_ENABLE;
        hdma_adc2.Init.PeriphDataAlignment   = DMA_PDATAALIGN_HALFWORD;
        hdma_adc2.Init.MemDataAlignment      = DMA_MDATAALIGN_WORD;
        hdma_adc2.Init.Mode                  = DMA_CIRCULAR;
        hdma_adc2.Init.Priority              = DMA_PRIORITY_HIGH;
        hdma_adc2.Init.FIFOMode              = DMA_FIFOMODE_DISABLE;
        hdma_adc2.Init.FIFOThreshold         = DMA_FIFO_THRESHOLD_HALFFULL;
        hdma_adc2.Init.MemBurst              = DMA_MBURST_SINGLE;
        hdma_adc2.Init.PeriphBurst           = DMA_PBURST_SINGLE;

        HAL_DMA_DeInit(&hdma_adc2);
        HAL_DMA_Init(&hdma_adc2);

        __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc2);

        HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, ADC_IRQPriority, 2);
        HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
    }
}

static int read_half[2] = {0,0};
uint16_t ADC_get(int chan){ // inversion & bipolar shaping
    // 1,2,3,1,2,3,4,5,6,4,5,6
    if(chan<3){
        chan += read_half[0] * ADC_CHANNELS;
    } else {
        chan -= ADC_CHANNELS; // 3-5 down to 0-2
        chan += read_half[1] * ADC_CHANNELS; // +0 or +3 -> 0-2 / 3-5
        chan += ADC_CHANNELS * 2; // += 6 -> 6-8 / 9-11
    }
    return adc_raw[chan];
}


//// private
void DMA2_Stream4_IRQHandler(void){
    HAL_DMA_IRQHandler(AdcHandle.DMA_Handle);
}

void DMA2_Stream2_IRQHandler(void){
    HAL_DMA_IRQHandler(AdcHandle2.DMA_Handle);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc){
    if(hadc == &AdcHandle){
        read_half[0] = 0;
    } else if(hadc == &AdcHandle2){
        read_half[1] = 0;
    }
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    if(hadc == &AdcHandle){
        read_half[0] = 1;
    } else if(hadc == &AdcHandle2){
        read_half[1] = 1;
    }
}
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc){
    Caw_printf("adc error\n\r");
}
