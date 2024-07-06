#pragma once

#include <stm32f7xx.h>
#include "interrupts.h" // DAC_IRQPriority

typedef enum{
    DAC_Stepped,
    DAC_Stepped_Fine,
    DAC_V8_Trim,
    DAC_V8_Offset,
    DAC_Fold1,
    DAC_Fold2,
    DAC_Fold3,
    DAC_Fold4,
    DAC_Fold5,
    DAC_Fold6,
    DAC_Fold11,
    DAC_Fold12,
    DAC_Fold10,
    DAC_Fold9,
    DAC_Fold8,
    DAC_Fold7
} DAC_Channels;

void DAC_Init( uint16_t bsize, uint8_t chan_count );
void DAC_Start(void);

int DAC_get_channel_id(int channel);

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
