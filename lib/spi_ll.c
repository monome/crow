#include "spi_ll.h"

#include <stm32f4xx.h>

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    if(hspi == &dac_spi){
        DAC_SPI_MspInit(hspi);
    } else {
        ADC_SPI_MspInit(hspi);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if(hspi == &dac_spi){
        DAC_SPI_MspDeInit(hspi);
    } else {
        ADC_SPI_MspDeInit(hspi);
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    ADC_SPI_TxRxCpltCallback(hspi);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    DAC_SPI_TxCpltCallback(hspi);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &dac_spi){
        DAC_SPI_ErrorCallback(hspi);
    } else {
        ADC_SPI_ErrorCallback(hspi);
    }
}
