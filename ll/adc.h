#pragma once

#include <stm32f7xx.h>

typedef enum{
    ADC_Fold,
    ADC_ID,
    ADC_Offset,
    ADC_Density,
    ADC_Steps,
    ADC_Rotate
} ADC_Channels;

void ADC_Init(void);

uint16_t ADC_get(int chan);

// hardware callback integrations
void DMA2_Stream4_IRQHandler(void);
void DMA2_Stream2_IRQHandler(void);
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);
