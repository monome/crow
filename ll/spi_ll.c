#include "spi_ll.h"

#include <stm32f7xx.h>

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    ADC_SPI_MspInit(hspi);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    ADC_SPI_MspDeInit(hspi);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    ADC_SPI_TxRxCpltCallback(hspi);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    ADC_SPI_ErrorCallback(hspi);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    ADC_SPI_RxCpltCallback(hspi);
}
