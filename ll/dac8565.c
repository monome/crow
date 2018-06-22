#include "dac8565.h"

#include "debug_usart.h"
#include "debug_pin.h"

#define DAC_BLOCK_SIZE   1
#define DAC_CHAN_COUNT   4
#define DAC_BUFFER_COUNT 2 // ping-pong
#define DAC_BUFFER_SIZE  ( DAC_CHAN_COUNT \
                         * DAC_BLOCK_SIZE \
                         * DAC_BUFFER_COUNT)
uint32_t i2s_buf[DAC_BUFFER_SIZE];

I2S_HandleTypeDef dac_i2s;

typedef struct{
    uint32_t samples[DAC_BUFFER_COUNT][DAC_BLOCK_SIZE][DAC_CHAN_COUNT];
} DAC_State_t;

DAC_State_t DAC_state;
uint32_t DAC_Init(void)
{
    // Set the SPI parameters
    dac_i2s.Instance          = I2Sx;
    dac_i2s.Init.Mode         = I2S_MODE_MASTER_TX;
    dac_i2s.Init.Standard     = I2S_STANDARD_PCM_SHORT;
    dac_i2s.Init.DataFormat   = I2S_DATAFORMAT_32B; // need 32b?
    dac_i2s.Init.MCLKOutput   = I2S_MCLKOUTPUT_ENABLE;
    dac_i2s.Init.AudioFreq    = I2S_AUDIOFREQ_192K;
    dac_i2s.Init.CPOL         = I2S_CPOL_LOW; // or _HIGH
    dac_i2s.Init.ClockSource  = I2S_CLOCK_SYSCLK; // or _PLL

    if(HAL_I2S_Init(&dac_i2s) != HAL_OK){ U_PrintLn("i2s_init"); }

    // NRST & NSS both high
    HAL_GPIO_WritePin( I2Sx_NRST_GPIO_PORT, I2Sx_NRST_PIN, 0 );
    volatile uint32_t t=10000; while(t){t--;}
    HAL_GPIO_WritePin( I2Sx_NRST_GPIO_PORT, I2Sx_NRST_PIN, 1 );

    return DAC_BLOCK_SIZE;
}

/* Initialize the DMA buffer to contain required metadata
 * Each channel is initialized with its relevant SPI command
 * The last channel latches all the new values to the outputs
 *
 * */
void DAC_Start(void)
{
    // prepare SPI packet metadata
    uint32_t* dbuf = DAC_state.samples[0][0];
    for( uint16_t i=0; i<DAC_BUFFER_SIZE; i++ ){
        uint8_t* addr_ptr = ((uint8_t*)dbuf) + 1;
        *addr_ptr = DAC8565_SET_ONE | 1<<1;
        /**addr_ptr = DAC8565_SET_ONE
                  | (i%4) << 1;*/
        /**addr_ptr = (i%4 == 3)
                        ? DAC8565_SET_ONE  | 3 << 1 // ch4 triggers update
                        : DAC8565_PREP_ONE | (i%4) << 1;
                        */
        dbuf++;
    }

    // set all channels to 0v (0xAAAA)
    dbuf = DAC_state.samples[0][0];
    for( uint16_t i=0; i<DAC_BUFFER_SIZE; i++ ){
        //uint8_t* addr_ptr = ((uint8_t*)dbuf) + 1;
        uint8_t* addr_ptr = ((uint8_t*)dbuf) + 0;
        *addr_ptr = 0xaa;
        addr_ptr = ((uint8_t*)dbuf) + 3;
        *addr_ptr = 0xaa;
        dbuf++;
    }





    // step in 2s for two half-words
/*    uint8_t* _buf = (uint8_t*)i2s_buf;
    for( uint16_t i=0; i<DAC_BUFFER_SIZE; i++ ){
        // set channel 1 to midpoint (2v5)
        //_buf++; // padding. can change to DATAFORMAT_24B?
        *_buf++ = 0xab; // BYTE1
        *_buf++ = (DAC8565_SET_ONE | 0x02); // BYTE0
        // *_buf++ = 0x1F << 0;//0x7F; // BYTE2
        *_buf++ = 0x00; // BYTE3
        *_buf++ = 0x00; // BYTE3
    }
    */
    HAL_I2S_Transmit_DMA( &dac_i2s
                        //, (uint16_t*)i2s_buf
                        , (uint16_t*)(DAC_state.samples[0][0])
                        , DAC_BUFFER_SIZE // sending as u16
                        );
}

typedef struct {
    uint8_t  dirty;
    uint8_t  cmd;
    uint16_t data;
} DAC_Samp_t;

DAC_Samp_t dac_buf[5];

void DAC_Update( void )
{
    /*
    // Check NSS is high, indicating no ongoing SPI comm'n
    if( HAL_GPIO_ReadPin( SPId_NSS_GPIO_PORT, SPId_NSS_PIN ) == GPIO_PIN_SET ){

        uint8_t ch; // Choose which channel to update
        if( dac_buf[4].dirty ){ ch = 4; } // Prioritize ALL
        else {
            static uint8_t last_ch = 0;
            uint8_t i=0;
            while(++i){ // Start at 1
                if( i==5 ){ return; } // All channels clean
                ch = (i+last_ch)%4;
                if( dac_buf[ch].dirty ){ break; } // Proceed to send
            } last_ch = ch;
        }
        // pull !SYNC low
        HAL_GPIO_WritePin( SPId_NSS_GPIO_PORT, SPId_NSS_PIN, 0 );
        Debug_Pin_Set( 1 );
        if(HAL_SPI_Transmit( &dac_spi
                               , (uint8_t*)&(dac_buf[ch].cmd)
                               , 3
                               , 10000 // timeout?
                               ) != HAL_OK ){
            U_PrintLn("spi_tx_fail");
        }
        DAC_SPI_TxCpltCallback( &dac_spi );
        Debug_Pin_Set( 0 );
        dac_buf[ch].dirty = 0; // Unmark dirty to avoid repeat send
    }
    */
}
void DAC_SetU16( int8_t channel, uint16_t value )
{
    value = (value & 0xFF)<<8 | (value & 0xFF00)>>8;
    if(channel >= 4){ return; } // invalid channel selected
    else if( channel < 0 ){
        dac_buf[4].dirty = 1;
        dac_buf[4].cmd   = (channel == -1)
                                ? DAC8565_SET_ALL
                                : DAC8565_REFRESH_ALL;
        dac_buf[4].data  = value;
        for( uint8_t i=0; i<4; i++ ){
            // individual buffers are aware of global changes
            dac_buf[i].dirty = 0;
            dac_buf[i].data  = dac_buf[4].data;
        }
    } else {
        if( value == dac_buf[channel].data ){ return; } // discard unchanged
        dac_buf[channel].dirty = 1;
        dac_buf[channel].cmd   = DAC8565_SET_ONE  // update & set output
                               | (channel<<1)
                               ;
        dac_buf[channel].data  = value;
    }
}

void HAL_I2S_TxHalfCpltCallback( I2S_HandleTypeDef *hi2s )
{
    /*
    // step in 2s for two half-words
    uint8_t* _buf = (uint8_t*)i2s_buf;
    for( uint16_t i=0; i<DAC_BLOCK_SIZE; i++ ){
        // set channel 1 to midpoint (2v5)
        //_buf++; // padding. can change to DATAFORMAT_24B?
        *_buf++ = 0xab; // BYTE1
        *_buf++ = (DAC8565_SET_ONE | 0x02); // BYTE0
        // *_buf++ = 0x1F << 0;//0x7F; // BYTE2
        *_buf++ = 0x00; // BYTE3
        *_buf++ = 0x00; // BYTE3
    }
    */

    /*HAL_I2S_Transmit_DMA( hi2s
                        , &i2s_buf[DAC_BLOCK_SIZE]
                        , DAC_BLOCK_SIZE
                        );*/
}
void HAL_I2S_TxCpltCallback( I2S_HandleTypeDef *hi2s )
{
    /*
    // step in 2s for two half-words
    uint8_t* _buf = (uint8_t*)(&i2s_buf[DAC_BLOCK_SIZE]);
    for( uint16_t i=0; i<DAC_BLOCK_SIZE; i++ ){
        // set channel 1 to midpoint (2v5)
        //_buf++; // padding. can change to DATAFORMAT_24B?
        *_buf++ = 0xcb; // BYTE1
        *_buf++ = (DAC8565_SET_ONE | 0x02); // BYTE0
        // *_buf++ = 0x1F << 0;//0x7F; // BYTE2
        *_buf++ = 0x00; // BYTE3
        *_buf++ = 0x00; // BYTE3
    }
    */

    /*HAL_I2S_Transmit_DMA( hi2s
                        , i2s_buf
                        , DAC_BLOCK_SIZE
                        );*/
}
void HAL_I2S_ErrorCallback( I2S_HandleTypeDef *hi2s )
{
    U_PrintLn("i2s_tx_error");
}
void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s)
{
    // I2S PLL Config
    RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;

    // PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) × (PLLI2SN/PLLM)
    // I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR
    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
    RCC_ExCLKInitStruct.I2sClockSelection = RCC_I2SCLKSOURCE_PLLI2S;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 384;
    RCC_ExCLKInitStruct.PLLI2S.PLLI2SR = 2;
    HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);

    // SPI/I2S & DMA
    I2Sx_CLK_ENABLE();
    I2Sx_DMAx_CLK_ENABLE();

    // GPIO pins
    I2Sx_NSS_GPIO_CLK_ENABLE();
    I2Sx_SCK_GPIO_CLK_ENABLE();
    I2Sx_MOSI_GPIO_CLK_ENABLE();
    I2Sx_NRST_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef  GPIO_InitStruct;

    GPIO_InitStruct.Pin       = I2Sx_SCK_PIN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = I2Sx_SCK_AF;
    HAL_GPIO_Init(I2Sx_SCK_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = I2Sx_MOSI_PIN;
    GPIO_InitStruct.Alternate = I2Sx_MOSI_AF;
    HAL_GPIO_Init(I2Sx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = I2Sx_NSS_PIN;
    GPIO_InitStruct.Alternate = I2Sx_NSS_AF;
    HAL_GPIO_Init(I2Sx_NSS_GPIO_PORT, &GPIO_InitStruct);

    // NRST handled manually
    GPIO_InitStruct.Pin       = I2Sx_NRST_PIN;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(I2Sx_NRST_GPIO_PORT, &GPIO_InitStruct);

    // DMA Streams
    static DMA_HandleTypeDef hdma_tx;

    hdma_tx.Instance                 = I2Sx_TX_DMA_STREAM;

    hdma_tx.Init.Channel             = I2Sx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hdma_tx.Init.Mode                = DMA_CIRCULAR;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_tx);

    // Associate I2S w/ DMA
    __HAL_LINKDMA(hi2s, hdmatx, hdma_tx);

    // DMA Priority (should be below IO, but above main process)
    HAL_NVIC_SetPriority(I2Sx_DMA_TX_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(I2Sx_DMA_TX_IRQn);

    // Must be lower priority than the above DMA
    HAL_NVIC_SetPriority(I2Sx_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(I2Sx_IRQn);
}

void DAC_I2S_MspDeInit(I2S_HandleTypeDef *hi2s)
{

  static DMA_HandleTypeDef hdma_tx;

  I2Sx_FORCE_RESET();
  I2Sx_RELEASE_RESET();

  HAL_GPIO_DeInit(I2Sx_NSS_GPIO_PORT,  I2Sx_NSS_PIN);
  HAL_GPIO_DeInit(I2Sx_SCK_GPIO_PORT,  I2Sx_SCK_PIN);
  HAL_GPIO_DeInit(I2Sx_MOSI_GPIO_PORT, I2Sx_MOSI_PIN);

  HAL_DMA_DeInit(&hdma_tx);

  HAL_NVIC_DisableIRQ(I2Sx_DMA_TX_IRQn);
  HAL_NVIC_DisableIRQ(I2Sx_IRQn);
}

void I2Sx_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(dac_i2s.hdmatx);
}
void I2Sx_IRQHandler(void)
{
    HAL_I2S_IRQHandler(&dac_i2s);
}
