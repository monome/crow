#pragma once

#include <stm32f7xx.h>

void ADC_Init(void);

void ADC_process(void); // call from main loop? better to just run a DMA w/ mux switching

float ADC_get(int chan);

void DMA2_Stream4_IRQHandler(void);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle);
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);
