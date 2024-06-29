#pragma once

#include <stm32f7xx.h>
#include "interrupts.h" // DAC_IRQPriority

void DAC_Init( uint16_t bsize, uint8_t chan_count );
void DAC_Start(void);

void DAC_PickleBlock( uint32_t* dac_pickle_ptr
                    , float*    unpickled_data
                    , uint16_t  bsize
                    );

// new
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai);

void DMA2_Stream6_IRQHandler(void);
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai);
