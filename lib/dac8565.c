#include "dac8565.h"

#include "debug_usart.h"

static SPI_HandleTypeDef dac_spi;
uint8_t aTxBuffer[64];

void DAC_Init(void)
{
    // Set the SPI parameters
    dac_spi.Instance               = SPIx;
    dac_spi.Init.Mode              = SPI_MODE_MASTER;
    dac_spi.Init.Direction         = SPI_DIRECTION_1LINE;
    dac_spi.Init.DataSize          = SPI_DATASIZE_8BIT;
    dac_spi.Init.CLKPolarity       = SPI_POLARITY_HIGH; // or _LOW?
    dac_spi.Init.CLKPhase          = SPI_PHASE_1EDGE;
    dac_spi.Init.NSS               = SPI_NSS_SOFT; //_HARD_OUTPUT
    dac_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    dac_spi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    dac_spi.Init.TIMode            = SPI_TIMODE_DISABLE;
    dac_spi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    dac_spi.Init.CRCPolynomial     = 7;

    if(HAL_SPI_Init(&dac_spi) != HAL_OK){ Debug_USART_printf("spi_init\n\r"); }
    else{ Debug_USART_printf("!!\n\r"); }

    // NRST & NSS both high
    HAL_GPIO_WritePin( SPIx_NSS_GPIO_PORT, SPIx_NSS_PIN, 1 );
    HAL_GPIO_WritePin( SPIx_NRST_GPIO_PORT, SPIx_NRST_PIN, 1 );
}

void DAC_Set(uint8_t channel, uint16_t value)
{
    static uint8_t aTxBuffer[3];

    //aTxBuffer[0] = channel;
    aTxBuffer[0] = 0b00110100;
    aTxBuffer[1] = (uint8_t)((value & 0xFF00) >>8);
    aTxBuffer[2] = (uint8_t)( value & 0xFF);

    // pull !SYNC low
    HAL_GPIO_WritePin( SPIx_NSS_GPIO_PORT, SPIx_NSS_PIN, 0 );
    if(HAL_SPI_Transmit_DMA( &dac_spi
                           , (uint8_t*)aTxBuffer
                           , 3
                           ) != HAL_OK ){
        Debug_USART_printf("spi_tx_fail\n\r");
    }

    // just wait til it's done (for now)
    while (HAL_SPI_GetState(&dac_spi) != HAL_SPI_STATE_READY){;;}
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    // signal end of transmission by pulling NSS high
    HAL_GPIO_WritePin( SPIx_NSS_GPIO_PORT, SPIx_NSS_PIN, 1 );
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    Debug_USART_printf("spi_error\n\r");
    // pull NSS high to cancel any ongoing transmission
    HAL_GPIO_WritePin( SPIx_NSS_GPIO_PORT, SPIx_NSS_PIN, 1 );
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    static DMA_HandleTypeDef hdma_tx;

    GPIO_InitTypeDef  GPIO_InitStruct;

    SPIx_NSS_GPIO_CLK_ENABLE();
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    SPIx_NRST_GPIO_CLK_ENABLE();

    SPIx_CLK_ENABLE();

    SPIx_DMAx_CLK_ENABLE();

    // GPIO pins
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    // NSS & NRST handled manually
    GPIO_InitStruct.Pin       = SPIx_NSS_PIN;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(SPIx_NSS_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = SPIx_NRST_PIN;
    HAL_GPIO_Init(SPIx_NRST_GPIO_PORT, &GPIO_InitStruct);

    // DMA Streams
    hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;

    hdma_tx.Init.Channel             = SPIx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_tx);

    // Associate SPI w/ DMA
    __HAL_LINKDMA(hspi, hdmatx, hdma_tx);

    // DMA Priority (should be below IO, but above main process)
    HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);

    // Must be lower priority than the above DMA
    HAL_NVIC_SetPriority(SPIx_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{

  static DMA_HandleTypeDef hdma_tx;

  SPIx_FORCE_RESET();
  SPIx_RELEASE_RESET();

  HAL_GPIO_DeInit(SPIx_NSS_GPIO_PORT, SPIx_NSS_PIN);
  HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
  HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);

  HAL_DMA_DeInit(&hdma_tx);

  HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);
  HAL_NVIC_DisableIRQ(SPIx_IRQn);
}

void SPIx_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(dac_spi.hdmatx);
}
void SPIx_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&dac_spi);
}
