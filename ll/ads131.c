#include "ads131.h"

#include <stm32f7xx_hal.h>

#include "debug_usart.h"
#include "debug_pin.h"

#define ADC_FRAMES     3 // status word, plus 2 channels
#define ADC_BUF_SIZE   (ADS_DATAWORDSIZE * ADC_FRAMES)
//#define ADC_BUF_SIZE   (ADC_FRAMES)

#define NSS_DELAY 10000
#define DELAY_usec(u) \
    do{ for( volatile int i=0; i<u; i++ ){;;} \
    } while(0);

SPI_HandleTypeDef adc_spi;
TIM_HandleTypeDef adc_tim;
TIM_OC_InitTypeDef tim_config;
uint16_t aRxBuffer[ADC_BUF_SIZE];
uint16_t aTxBuffer[ADC_BUF_SIZE];

uint8_t  adc_count = 2;
uint32_t adc_samp_count = 64;

void ADS_Init_Sequence(void);
void ADS_Reset_Device(void);
uint8_t ADS_IsReady( void );
uint16_t ADS_Cmd( uint16_t command );
uint16_t ADS_Reg( uint8_t reg_mask, uint8_t val );
void ADC_TxRx( uint16_t* aTxBuffer, uint16_t* aRxBuffer, uint32_t size );
void ADC_Rx( uint16_t* aRxBuffer, uint32_t size );
uint8_t _ADC_CheckErrors( uint16_t error_mask );
//uint8_t _ADC_CheckErrors( uint16_t expect, uint16_t error );

void ADC_Init( uint16_t bsize, uint8_t chan_count )
{
    adc_count  = chan_count;
    adc_samp_count = bsize * chan_count;

    // Set the SPI parameters
    adc_spi.Instance               = SPIa;
    adc_spi.Init.Mode              = SPI_MODE_MASTER;
    adc_spi.Init.Direction         = SPI_DIRECTION_2LINES;
    adc_spi.Init.DataSize          = SPI_DATASIZE_16BIT;
    adc_spi.Init.CLKPolarity       = SPI_POLARITY_LOW;
    adc_spi.Init.CLKPhase          = SPI_PHASE_2EDGE;
    adc_spi.Init.NSS               = SPI_NSS_SOFT;
    adc_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128; // 1.2MHz
    adc_spi.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    adc_spi.Init.TIMode            = SPI_TIMODE_DISABLE;
    adc_spi.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    adc_spi.Init.CRCPolynomial     = 7;
    if(HAL_SPI_Init(&adc_spi) != HAL_OK){ U_PrintLn("spi_init"); }

    //uint32_t prescaler = (uint32_t)((SystemCoreClock / 10000)-1);
    uint32_t period_value = 0x06; // ~8.3MHz
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
    if(HAL_TIM_PWM_Start( &adc_tim, TIMa_CHANNEL ) != HAL_OK){
        U_PrintLn("tim_st");
    }

    for( uint8_t i=0; i<ADC_BUF_SIZE; i++ ){
        aRxBuffer[i] = 0;
        aTxBuffer[i] = 0;
    }
    ADS_Init_Sequence();
}

void ADS_Init_Sequence(void)
{
    ADS_Reset_Device();

    ADS_Cmd(ADS_UNLOCK);
    if( ADS_Cmd(ADS_NULL) != ADS_UNLOCK ){ U_PrintLn("Can't Unlock"); }

    // CONFIGURE REGS HERE
    ADS_Reg(ADS_WRITE_REG | ADS_CLK1, 0x01 << 1); // MCLK/8 // 0x02 before
    //U_PrintU16( ADS_Cmd(ADS_NULL) ); // assert ADS_CLK1
        // 0x02 is /2 which is least divisions (fastest internal)
        // meaning slowest clkout from uc and least noise?
        // CLKIN divider. see p63 of ads131 datasheet
    //  clk2 sets ADC read rate?
    ADS_Reg(ADS_WRITE_REG | ADS_CLK2, 0x2a); // MCLK/8
    // this was arrived at experimentally
    // 0x2* sets clockdiv to /2 (the least divisions, for slowest MCLK)
    // 0x*b was lowest (ie most oversampling) where -5v actually reached 0x8000
    // using 0x*c for safety
    // fMOD = fICLK / 8, and OSR=400 (default 0x86)
        // see p64 & Table30(p65) for OSR settings
        // Table30 shows effective sampling rates w/ diff MCLKs
        // start at 1kHz, then work up depending on quality/cpu
    // fMOD = fICLK / 2  fICLK = fCLKIN / 2048 ** now is 500hz ** 0x21
    ADS_Reg(ADS_WRITE_REG | ADS_ADC_ENA, 0x03); // this contradicts datasheet
        // DS suggests 0 or 0xF, but we use 0x03 for only 2 channels
        // this is based on making certain the ADS_Reg response is correct
    ADS_Cmd(ADS_WAKEUP);

    ADS_Cmd(ADS_LOCK);
}
void ADS_Reset_Device(void)
{
    ADS_IsReady(); // send query
    if(ADS_IsReady()){ return; } // ready to go!

    // not ready: reset
    HAL_GPIO_WritePin( SPIa_NRST_GPIO_PORT, SPIa_NRST_PIN, 0 );
    HAL_Delay(5); // reset can take up to 4.5ms (p35)
    HAL_GPIO_WritePin( SPIa_NRST_GPIO_PORT, SPIa_NRST_PIN, 1 );
    HAL_Delay(5); // reset can take up to 4.5ms (p35)

    while(!ADS_IsReady()){} // needs timeout
    ADS_Cmd(ADS_NULL);
}
uint16_t ADS_Reg( uint8_t reg_mask, uint8_t val )
{
    uint16_t retval = 0;

    if( (reg_mask & ADS_READ_REG) == ADS_READ_REG ){ // read
        ADS_Cmd( ((uint16_t)reg_mask) << 8);         // send query
        retval = ADS_Cmd( ADS_NULL );                // get response
    } else { // write
        retval = ADS_Cmd( ((uint16_t)reg_mask) << 8 | val );
        // write success assertion
        uint16_t status = ADS_Cmd(ADS_NULL);
        uint16_t expect = ( ((reg_mask & 0x1F) | 0x20)<<8
                          | val
                          );
        if( status != expect ){
            U_Print("!WREG expect: "); U_PrintU16(expect);
            U_Print("received: "); U_PrintU16(status);
        }
    }
    return retval;
}

uint8_t ADS_IsReady( void )
{
    uint32_t* tx = (uint32_t*)aTxBuffer;
    uint32_t* rx = (uint32_t*)aRxBuffer;
    *tx++ = 0;
    *rx++ = 0;
    ADC_Rx( aRxBuffer, 2 );

    return (aRxBuffer[0] == ADS_READY);
}

uint16_t ADS_Cmd( uint16_t command )
{
    aTxBuffer[0] = command;
    aRxBuffer[0] = 0;
    ADC_TxRx( aTxBuffer, aRxBuffer, 1);
    
    return aRxBuffer[0];
}
uint16_t ADC_GetU16( uint8_t channel )
{
    //uint32_t* tx = (uint32_t*)aTxBuffer;
    //*tx = 0; // NULL command
    //aTxBuffer[0] = 0;
    //ADC_TxRx( aTxBuffer, aRxBuffer, ADC_BUF_SIZE );

    /*if( _ADC_CheckErrors( aRxBuffer[0] ) ){
        U_PrintLn("");
    }
    */

    //U_PrintU16(aRxBuffer[0]);
    return aRxBuffer[channel+1];
}

#define ADC_U16_TO_V        ((float)(15.0 / 65535.0))

static float last[2] = {0.0,0.0};
void ADC_UnpickleBlock( float*   unpickled
                      , uint16_t bsize
                      )
{
    float* unpick = unpickled;
    // Return current buf
    for( uint8_t j=1; j<=adc_count; j++ ){
        // cast to signed -> cast to float -> scale -> shift
        float once = ((float)((int16_t*)aRxBuffer)[j]) * ADC_U16_TO_V + 2.5;
        float c    = 1.0 / bsize;
        for( uint16_t i=0; i<bsize; i++ ){
            *unpick++ = last[j-1] + (c * (i+1))*(once - last[j-1]);
        }
        last[j-1] = once;
    }


    // Request next buffer (if driver is free)
    if (HAL_SPI_GetState(&adc_spi) == HAL_SPI_STATE_READY){
        HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 0 );
//TODO: set a timer and wait in the background
        DELAY_usec(8000); // can't seem to go lower
        uint32_t* tx = (uint32_t*)aTxBuffer;
        *tx = 0; // NULL command
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
        if(HAL_SPI_TransmitReceive_DMA( &adc_spi
                                      , (uint8_t*)aTxBuffer
                                      , (uint8_t*)aRxBuffer
                                      , ADC_BUF_SIZE
                                      ) != HAL_OK ){
            U_PrintLn("spi_txrx_fail");
        }
__set_PRIMASK( old_primask );
    }
}

float ADC_GetValue( uint8_t channel )
{
    return last[channel];
}

void ADC_Rx( uint16_t* aRxBuffer, uint32_t size )
{
    // pull !SYNC low
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 0 );
    DELAY_usec(NSS_DELAY);
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
    if(HAL_SPI_Receive_DMA( &adc_spi
                          , (uint8_t*)aRxBuffer
                          , size
                          ) != HAL_OK ){
        U_PrintLn("spi_rx_fail");
    }
__set_PRIMASK( old_primask );
    // just wait til it's done (for now)
    while (HAL_SPI_GetState(&adc_spi) != HAL_SPI_STATE_READY){;;}
}

void ADC_TxRx( uint16_t* aTxBuffer, uint16_t* aRxBuffer, uint32_t size )
{
    // pull !SYNC low
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 0 );
    DELAY_usec(NSS_DELAY);
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
    if(HAL_SPI_TransmitReceive_DMA( &adc_spi
                                  , (uint8_t*)aTxBuffer
                                  , (uint8_t*)aRxBuffer
                                  , size
                                  ) != HAL_OK ){
        U_PrintLn("spi_txrx_fail");
    }
__set_PRIMASK( old_primask );
    // just wait til it's done (for now)
    while (HAL_SPI_GetState(&adc_spi) != HAL_SPI_STATE_READY){;;}
}

uint8_t _ADC_CheckErrors( uint16_t error )
{
    uint8_t retval = 0;
    //error = error >> 8;
    if( error != 0 ){ // fault handler
        retval = 1;
        if( error & 0x2 ){} // Data ready fault
        if( error & 0x4 ){ // Resync fault
            //U_PrintLn("sync");
        }
        if( error & 0x8 ){ // Watchdog timer
            U_PrintLn("watchdog");
        }
        if( error & 0x10 ){ // ADC input fault
            //U_Print("adc_p: ");
            //U_PrintU16(ADS_Reg( ADS_READ_REG | ADS_STAT_P, 0 ));
            //U_Print("adc_n: ");
            //U_PrintU16(ADS_Reg( ADS_READ_REG | ADS_STAT_N, 0 ));
        }
        if( error & 0x20  ){ // SPI fault
            //U_Print("spi: ");
            //U_PrintU16(ADS_Reg( ADS_READ_REG | ADS_STAT_S, 0 ));
        }
        if( error & 0x40 ){ // Command fault
            //U_PrintLn("bad_cmd");
            return 2;
        }
    }
    return retval;
}
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    U_PrintLn("spi_error");
    // pull NSS high to cancel any ongoing transmission
    //DELAY_usec(NSS_DELAY);
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 1 );
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    //U_PrintLn("txrx-cb");
    // signal end of transmission by pulling NSS high
    //DELAY_usec(NSS_DELAY);
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 1 );
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    //U_PrintLn("rx-cb");
    // signal end of transmission by pulling NSS high
    //_ADC_CheckErrors( aRxBuffer[0] );
    //DELAY_usec(NSS_DELAY);
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 1 );
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    static DMA_HandleTypeDef hdma_rx;
    static DMA_HandleTypeDef hdma_tx;

    GPIO_InitTypeDef  GPIO_InitStruct;

    SPIa_NSS_GPIO_CLK_ENABLE();
    SPIa_SCK_GPIO_CLK_ENABLE();
    SPIa_MISO_GPIO_CLK_ENABLE();
    SPIa_MOSI_GPIO_CLK_ENABLE();

    SPIa_NRST_GPIO_CLK_ENABLE();

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

    GPIO_InitStruct.Pin       = SPIa_NRST_PIN;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(SPIa_NRST_GPIO_PORT, &GPIO_InitStruct);

    // pull NSS high immediately (resting state)
    HAL_GPIO_WritePin( SPIa_NRST_GPIO_PORT, SPIa_NRST_PIN, 1 );
    HAL_GPIO_WritePin( SPIa_NSS_GPIO_PORT, SPIa_NSS_PIN, 1 );

    // DMA Streams
    hdma_tx.Instance                 = SPIa_TX_DMA_STREAM;

    hdma_tx.Init.Channel             = SPIa_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
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
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
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
    HAL_NVIC_SetPriority( SPIa_DMA_TX_IRQn
                        , SPIa_DMA_TX_IRQPriority
                        , SPIa_DMA_TX_IRQSubPriority
                        );
    HAL_NVIC_EnableIRQ( SPIa_DMA_TX_IRQn );

    HAL_NVIC_SetPriority( SPIa_DMA_RX_IRQn
                        , SPIa_DMA_RX_IRQPriority
                        , SPIa_DMA_RX_IRQSubPriority
                        );
    HAL_NVIC_EnableIRQ( SPIa_DMA_RX_IRQn );

    // Must be lower priority than the above DMA
    HAL_NVIC_SetPriority( SPIa_IRQn
                        , SPIa_IRQPriority
                        , SPIa_IRQSubPriority
                        );
    HAL_NVIC_EnableIRQ( SPIa_IRQn );
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
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
