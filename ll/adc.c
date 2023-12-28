#include "adc.h"

#include <stdio.h>
#include <stm32f7xx_hal.h> // HAL_Delay()

#include "interrupts.h"

#include "tp.h"

static ADC_HandleTypeDef AdcHandle;
static ADC_ChannelConfTypeDef sConfig;
static uint32_t mchan = 0; // mux channel (for controlling tp module)

#define ADC_CHANNELS 5
#define ADC_SEQUENCE_LEN 8
#define ADC_RAW_LEN (ADC_CHANNELS * ADC_SEQUENCE_LEN)
volatile uint32_t adc_raw[ADC_RAW_LEN];

static void start_conversion(int offset);

void ADC_Init(void)
{
    AdcHandle.Instance                   = ADC1;
    HAL_ADC_DeInit(&AdcHandle);

    AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
    AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
    AdcHandle.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    AdcHandle.Init.ContinuousConvMode    = ENABLE; // i think disable
    AdcHandle.Init.NbrOfConversion       = 5; // scan all 5 chans on ADC1
    AdcHandle.Init.DiscontinuousConvMode = DISABLE; // i think enable
    AdcHandle.Init.NbrOfDiscConversion   = 1;
    AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
    AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    AdcHandle.Init.DMAContinuousRequests = ENABLE;
    AdcHandle.Init.EOCSelection          = ADC_EOC_SEQ_CONV;

    if( HAL_ADC_Init( &AdcHandle ) != HAL_OK ){
        printf("HAL_ADC_Init failed\n");
    }

    // Channel configuration
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    sConfig.Offset       = 0;

    sConfig.Channel      = ADC_CHANNEL_0;
    sConfig.Rank         = 1;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_1;
    sConfig.Rank         = 2;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_2;
    sConfig.Rank         = 3;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_3;
    sConfig.Rank         = 4;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        printf("HAL_ADC_ConfigChannel failed\n");
    sConfig.Channel      = ADC_CHANNEL_4;
    sConfig.Rank         = 5;
    if( HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK )
        printf("HAL_ADC_ConfigChannel failed\n");

    start_conversion(0);
}

static void start_conversion(int offset){
    TP_adc_mux_1(mchan);
    if( HAL_ADC_Start_DMA( &AdcHandle
                         , (uint32_t*)&adc_raw[offset*ADC_CHANNELS]
                         , ADC_CHANNELS 
                         ) != HAL_OK ){
        printf("HAL_ADC_Start_DMA failed, retrying..\n");
        // HAL_Delay(10);
        if( HAL_ADC_Start_DMA( &AdcHandle
                             , (uint32_t*)&adc_raw[offset*ADC_CHANNELS]
                             , ADC_CHANNELS 
                             ) != HAL_OK ){
            printf("HAL_ADC_Start_DMA failed again, ignoring\n");
            return;
        }
    }
    // printf("conversion started\n\r");
}

static DMA_HandleTypeDef  hdma_adc;
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef          gpio;

    __HAL_RCC_ADC1_CLK_ENABLE(); // just using ADC1 for now
    // __HAL_RCC_ADC2_CLK_ENABLE();
    // __HAL_RCC_ADC3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE(); ////////////////// check
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pull = GPIO_NOPULL;

    // ADC1,2,3
    gpio.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
    HAL_GPIO_Init(GPIOA, &gpio);

    hdma_adc.Instance = DMA2_Stream4; /////////// check

    hdma_adc.Init.Channel               = DMA_CHANNEL_0; /////////// check
    hdma_adc.Init.Direction             = DMA_PERIPH_TO_MEMORY;
    hdma_adc.Init.PeriphInc             = DMA_PINC_DISABLE;
    hdma_adc.Init.MemInc                = DMA_MINC_ENABLE;
    hdma_adc.Init.PeriphDataAlignment   = DMA_PDATAALIGN_WORD;
    hdma_adc.Init.MemDataAlignment      = DMA_MDATAALIGN_WORD;
    hdma_adc.Init.Mode                  = DMA_NORMAL;
    hdma_adc.Init.Priority              = DMA_PRIORITY_HIGH;
    hdma_adc.Init.FIFOMode              = DMA_FIFOMODE_DISABLE;
    hdma_adc.Init.FIFOThreshold         = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_adc.Init.MemBurst              = DMA_MBURST_SINGLE;
    hdma_adc.Init.PeriphBurst           = DMA_PBURST_SINGLE;

    HAL_DMA_DeInit(&hdma_adc);
    HAL_DMA_Init(&hdma_adc);

    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

    // FIXME
     // (+++) Configure the priority and enable the NVIC for the transfer complete
     //             interrupt on the two DMA Streams. The output stream should have higher
     //             priority than the input stream.
    HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, ADC_IRQPriority, 1);
    HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);
}

// static float i12b = 1.0 / 4096.0;
float ADC_get(int chan){ // inversion & bipolar shaping
    // return (2.0 * adc_raw[chan] * i12b) - 1.0;
    return adc_raw[chan];
}


//// private
void DMA2_Stream4_IRQHandler(void){
    HAL_DMA_IRQHandler(AdcHandle.DMA_Handle);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle){
    // advance MUX
    mchan++;
    mchan &= 0x7; // wrap to 8 channels

    // start next conversion
    start_conversion(mchan);
    // printf("adc conv complete\n\r");
}
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc){
    printf("adc error\n\r");
}
