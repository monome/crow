#pragma once

#include <stm32f7xx.h>
#include "interrupts.h" // DAC_IRQPriority

void DAC_Init( uint16_t bsize, uint8_t chan_count );
void DAC_Start(void);

void DAC_CalibrateScalar( uint8_t channel, float scale );
void DAC_CalibrateOffset( uint8_t channel, float volts );
void DAC_PickleBlock( uint16_t* dac_pickle_ptr
                    , float*    unpickled_data
                    , uint16_t  bsize
                    );

void DAC_I2S_MspInit(I2S_HandleTypeDef *hi2s);
void DAC_I2S_MspDeInit(I2S_HandleTypeDef *hi2s);



// new
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai);

void DMA2_Stream3_IRQHandler(void);
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai);
