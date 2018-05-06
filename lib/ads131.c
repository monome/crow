#include "ads131.h"

#include "spi_ll.h"
#include "debug_usart.h"

#define ADC_DATAWIDTH  4 // 32bit
#define ADC_FRAMES     6 // status word, plus 2 channels
#define ADC_CMD_SIZE   4 // 32bit only
#define ADC_BUF_SIZE   (ADC_DATAWIDTH * ADC_FRAMES)

SPI_HandleTypeDef adc_spi;
TIM_HandleTypeDef adc_tim;
TIM_OC_InitTypeDef tim_config;
uint8_t aRxBuffer[ADC_BUF_SIZE];
uint8_t aTxBuffer[ADC_BUF_SIZE];

uint16_t ADC_Cmd(uint32_t command);
void ADC_TxRx( uint8_t* aTxBuffer, uint8_t* aRxBuffer, uint32_t size );
uint32_t _flip_byte_order(uint32_t word);

void ADC_Init(void)
{
    // Set the SPI parameters
    adc_spi.Instance               = SPIa;
    adc_spi.Init.Mode              = SPI_MODE_MASTER;
    adc_spi.Init.Direction         = SPI_DIRECTION_2LINES;
    adc_spi.Init.DataSize          = SPI_DATASIZE_8BIT;
    adc_spi.Init.CLKPolarity       = SPI_POLARITY_HIGH; // or _LOW?
    adc_spi.Init.CLKPhase          = SPI_PHASE_1EDGE;
    adc_spi.Init.NSS               = SPI_NSS_SOFT; //_HARD_OUTPUT
    adc_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; // ~120kHz
    adc_spi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    adc_spi.Init.TIMode            = SPI_TIMODE_DISABLE;
    adc_spi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    adc_spi.Init.CRCPolynomial     = 7;
    if(HAL_SPI_Init(&adc_spi) != HAL_OK){ U_PrintLn("spi_init"); }

    //uint32_t prescaler = (uint32_t)((SystemCoreClock / 10000)-1);
    uint32_t period_value = 20; // 2MHz w/ prescaler=1 @84MHz master
    adc_tim.Instance = TIMa;
    adc_tim.Init.Period = period_value;
    adc_tim.Init.Prescaler = 1; //prescaler;
    adc_tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    adc_tim.Init.CounterMode = TIM_COUNTERMODE_UP;
    if(HAL_TIM_PWM_Init(&adc_tim) != HAL_OK){ U_PrintLn("tim_init"); }

    tim_config.OCMode       = TIM_OCMODE_PWM1;
    tim_config.OCPolarity   = TIM_OCPOLARITY_HIGH;
    tim_config.OCFastMode   = TIM_OCFAST_DISABLE;
    tim_config.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    tim_config.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    tim_config.OCIdleState  = TIM_OCIDLESTATE_RESET;
    tim_config.Pulse        = period_value/2;
    if(HAL_TIM_PWM_ConfigChannel(&adc_tim, &tim_config, TIMa_CHANNEL) != HAL_OK){
        U_PrintLn("tim_config");
    }

    // Start MCLK & set NSS high
    if(HAL_TIM_PWM_Start( &adc_tim, TIMa_CHANNEL ) != HAL_OK){
        U_PrintLn("tim_st");
    }
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 1 );

    for( uint8_t i=0; i<ADC_BUF_SIZE; i++ ){
        aRxBuffer[i] = 0;
        aTxBuffer[i] = 0;
    }
    while(ADC_Cmd(ADS_NULL) != ADS_READY){
        // try a hardware reset here! we have NRST under uc control
    }
    //ADC_Cmd(ADS_RESET);
    ADC_Cmd(ADS_UNLOCK); // reset
}

uint16_t ADC_Cmd(uint32_t command)
{
    uint32_t* cmd = (uint32_t*)aTxBuffer;
    *cmd = command;
    ADC_TxRx( aTxBuffer, aRxBuffer, ADC_CMD_SIZE );
    U_PrintU32(_flip_byte_order(((uint32_t*)aRxBuffer)[0]));
    U_PrintLn("");

    return (uint16_t)(_flip_byte_order(((uint32_t*)aRxBuffer)[0])>>16);
}
uint32_t _flip_byte_order(uint32_t word)
{
    uint32_t retval = (word >> 24);
    retval |= (word >> 8) & 0x0000FF00;
    retval |= (word << 8) & 0x00FF0000;
    retval |= (word << 24) & 0xFF000000;
    return retval;
}
void ADC_Get( uint8_t channel )
{
    aTxBuffer[0] = 0x0;
    aTxBuffer[1] = 0x0;
    aTxBuffer[2] = 0;
    aTxBuffer[3] = 0;
    ADC_TxRx( aTxBuffer, aRxBuffer, ADC_BUF_SIZE );
    U_PrintU32(_flip_byte_order(((uint32_t*)aRxBuffer)[0]));
    U_PrintU32(_flip_byte_order(((uint32_t*)aRxBuffer)[1]));
    U_PrintU32(_flip_byte_order(((uint32_t*)aRxBuffer)[2]));
    U_PrintU32(_flip_byte_order(((uint32_t*)aRxBuffer)[3]));
    U_PrintU32(_flip_byte_order(((uint32_t*)aRxBuffer)[4]));
    U_PrintU32(_flip_byte_order(((uint32_t*)aRxBuffer)[5]));
    U_PrintLn("");
    //return (((int32_t*)aRxBuffer)[1]);
}
void ADC_TxRx( uint8_t* aTxBuffer, uint8_t* aRxBuffer, uint32_t size )
{
    // pull !SYNC low
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 0 );
    if(HAL_SPI_TransmitReceive_DMA( &adc_spi
                                  , aTxBuffer
                                  , aRxBuffer
                                  , size
                                  ) != HAL_OK ){
        U_PrintLn("spi_tx_fail");
    }
    // just wait til it's done (for now)
    while (HAL_SPI_GetState(&adc_spi) != HAL_SPI_STATE_READY){;;}
}
void ADC_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    U_PrintLn("spi_error");
    // pull NSS high to cancel any ongoing transmission
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 1 );
}

void ADC_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    // signal end of transmission by pulling NSS high
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 1 );
}

void ADC_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    static DMA_HandleTypeDef hdma_rx;
    static DMA_HandleTypeDef hdma_tx;

    GPIO_InitTypeDef  GPIO_InitStruct;

    SPIa_NSS_GPIO_CLK_ENABLE();
    SPIa_SCK_GPIO_CLK_ENABLE();
    SPIa_MISO_GPIO_CLK_ENABLE();
    SPIa_MOSI_GPIO_CLK_ENABLE();

    SPIa_CLK_ENABLE();

    SPIa_DMAx_CLK_ENABLE();

    // GPIO pins
    GPIO_InitStruct.Pin       = SPIa_SCK_PIN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = SPIa_SCK_AF;
    HAL_GPIO_Init(SPIa_SCK_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = SPIa_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIa_MISO_AF;
    HAL_GPIO_Init(SPIa_MISO_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = SPIa_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIa_MOSI_AF;
    HAL_GPIO_Init(SPIa_MOSI_GPIO_PORT, &GPIO_InitStruct);

    // NSS & NRST handled manually
    GPIO_InitStruct.Pin       = SPIa_NSS_PIN;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(SPIa_NSS_GPIO_PORT, &GPIO_InitStruct);

    // DMA Streams
    hdma_tx.Instance                 = SPIa_TX_DMA_STREAM;

    hdma_tx.Init.Channel             = SPIa_TX_DMA_CHANNEL;
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

    // Reception:
    hdma_rx.Instance                 = SPIa_RX_DMA_STREAM;

    hdma_rx.Init.Channel             = SPIa_RX_DMA_CHANNEL;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_rx);

    // Associate SPI w/ DMA
    __HAL_LINKDMA(hspi, hdmarx, hdma_rx);


    // DMA Priority (should be below IO, but above main process)
    HAL_NVIC_SetPriority(SPIa_DMA_TX_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(SPIa_DMA_TX_IRQn);

    HAL_NVIC_SetPriority(SPIa_DMA_RX_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIa_DMA_RX_IRQn);

    // Must be lower priority than the above DMA
    HAL_NVIC_SetPriority(SPIa_IRQn, 1, 2);
    HAL_NVIC_EnableIRQ(SPIa_IRQn);
}

void ADC_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    static DMA_HandleTypeDef hdma_rx;
    static DMA_HandleTypeDef hdma_tx;

    SPIa_FORCE_RESET();
    SPIa_RELEASE_RESET();

    HAL_GPIO_DeInit(SPIa_SCK_GPIO_PORT, SPIa_SCK_PIN);
    HAL_GPIO_DeInit(SPIa_MISO_GPIO_PORT, SPIa_MISO_PIN);
    HAL_GPIO_DeInit(SPIa_MOSI_GPIO_PORT, SPIa_MOSI_PIN);
    HAL_GPIO_DeInit(SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN);

    HAL_DMA_DeInit(&hdma_tx);
    HAL_DMA_DeInit(&hdma_rx);

    HAL_NVIC_DisableIRQ(SPIa_DMA_TX_IRQn);
    HAL_NVIC_DisableIRQ(SPIa_DMA_RX_IRQn);
    HAL_NVIC_DisableIRQ(SPIa_IRQn);
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    TIMa_MCLK_GPIO_CLK_ENABLE();

    TIMa_CLK_ENABLE();

    GPIO_InitStruct.Pin       = TIMa_MCLK_PIN;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    //GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = TIMa_MCLK_AF;
    HAL_GPIO_Init(TIMa_MCLK_GPIO_PORT, &GPIO_InitStruct);
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef *htim)
{
    TIMa_FORCE_RESET();
    TIMa_RELEASE_RESET();

    HAL_GPIO_DeInit(TIMa_MCLK_GPIO_PORT, TIMa_MCLK_PIN);
}

void SPIa_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(adc_spi.hdmarx);
}
void SPIa_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(adc_spi.hdmatx);
}
void SPIa_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&adc_spi);
}
