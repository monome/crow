#include "dac108.h"

#include <stdio.h>
#include "stdlib.h" // malloc()
#include "debug_pin.h"

#include "adda.h"   // ADDA_BlockProcess()
#include "wrMath.h" // lim_f()

#define DAC_BUFFER_COUNT 2 // ping-pong

// pointer to the malloc()d buffer from DAC_Init()
uint16_t  samp_count = 0;
uint16_t* samples = NULL;

#define DAC_ZERO_VOLTS      ((uint16_t)(((uint32_t)0xFFFF * 2)/3))
#define DAC_V_TO_U16        ((float)(65535.0 / 15.0))
#define DAC_CHANNELSS 8
float dac_calibrated_offset[DAC_CHANNELSS];
float dac_calibrated_scalar[DAC_CHANNELSS];

static void sai_start_transmit(uint16_t* hwords, uint16_t count);
static void sai_init(void);

void DAC_Init(uint16_t bsize, uint8_t chan_count){
    sai_init();

    // Create the sample buffer for DMA transfer
    samp_count = DAC_BUFFER_COUNT * bsize * chan_count; // 512
    printf("samp_count %x\n\r", samp_count);
    samples = malloc( sizeof(uint16_t) * samp_count ); // 1k
    if(samples == NULL){ printf("!DAC_buffer\n"); }
    for( int i=0; i<samp_count; i++ ){ samples[i] = 0; } // unnecessary

    // for( int j=0; j<DAC_CHANNELSS; j++ ){
    //     dac_calibrated_offset[j] = 0.0;
    //     dac_calibrated_scalar[j] = DAC_V_TO_U16;
    // }
}

void DAC_Start(void){
    for(int i=0; i++; i<samp_count){ // double-buffered
        // NB: we add 1 to i, so we start the loop on second channel
        int chan_mod = (i+1) & (8-1); // channel modulo 8, running 1-7, then 0.
        if(chan_mod == 0){ // chan 0
            samples[i] = 0b1011 << 12; // sets chan 1 & updates all outputs
        } else { // chans 1-7
            samples[i] = chan_mod<<12; // sets channel select bits
        }
    }
    sai_start_transmit(samples, samp_count);
}

void DAC_CalibrateScalar( uint8_t channel, float scale ){
    dac_calibrated_scalar[channel] = DAC_V_TO_U16 * scale;
}

void DAC_CalibrateOffset( uint8_t channel, float volts ){
    dac_calibrated_offset[channel] = volts;
}

int32_t lim_i32_u16( int32_t v ){
    return (v > (int32_t)(uint16_t)0xFFFF) ? 0xFFFF : (v < (int32_t)0) ? 0 : v;
}

/* Does all the work converting a generic representation into serial packets
 * Convert floats (representing volts) to u16 representation
 * Interleave a block of each channel into a stream
 * */
void DAC_PickleBlock( uint16_t* dac_pickle_ptr
                    , float*    unpickled_data
                    , uint16_t  bsize
                    )
{
    // TODO calibration (maybe not)
    // for( uint8_t j=0; j<4; j++ ){
    //     add_vf_f( &(unpickled_data[j*bsize])
    //             , dac_calibrated_offset[j]
    //             , &(unpickled_data[j*bsize])
    //             , bsize
    //             );
    // }
    // for( uint8_t j=0; j<4; j++ ){
    //     mul_vf_f( &(unpickled_data[j*bsize])
    //             , dac_calibrated_scalar[j] // scale volts up to u16
    //             , bsize
    //             );
    // }
    
    for(int i=0; i<ADDA_DAC_CHAN_COUNT * bsize; i++){
        // ? = weird scan through the buffer to interleave channels
        int block_index = (i*bsize);
        int b_mod = block_index % ADDA_DAC_CHAN_COUNT;
        int b_div = block_index / ADDA_DAC_CHAN_COUNT;
        *dac_pickle_ptr = lim_i32_u16((int32_t)((1.0 + unpickled_data[b_mod + b_div]) * 16384.0)); // _ = scale from float representation up to integers for casting
        dac_pickle_ptr++;
    }
}

static SAI_HandleTypeDef hsai_a;
static DMA_HandleTypeDef hdma_tx_a;

static void sai_init(void){

    RCC_PeriphCLKInitTypeDef rcc;
    rcc.PeriphClockSelection    = RCC_PERIPHCLK_SAI1;
    rcc.Sai1ClockSelection      = RCC_SAI1CLKSOURCE_PLLSAI;

    // here we configure for 3.072MHz
    // ie 8 channels, 16bits, 24kHz sample rate
    rcc.PLLSAI.PLLSAIN          = 384;
    rcc.PLLSAI.PLLSAIQ          = 5;
    rcc.PLLSAIDivQ              = 25;
    // see @ciel/tools/sai_pll_calculator.lua to configure

    HAL_RCCEx_PeriphCLKConfig(&rcc);

    // Initialize SAI
    __HAL_SAI_RESET_HANDLE_STATE(&hsai_a);

    // block A
    hsai_a.Instance = SAI1_Block_A; // TODO follow instance
    __HAL_SAI_DISABLE(&hsai_a);
    hsai_a.Init.AudioMode         = SAI_MODEMASTER_TX;
    hsai_a.Init.Synchro           = SAI_ASYNCHRONOUS;
    hsai_a.Init.SynchroExt        = SAI_SYNCEXT_DISABLE;
    hsai_a.Init.OutputDrive       = SAI_OUTPUTDRIVE_ENABLE;
    hsai_a.Init.NoDivider         = SAI_MASTERDIVIDER_DISABLE;
    hsai_a.Init.FIFOThreshold     = SAI_FIFOTHRESHOLD_1QF;
    hsai_a.Init.AudioFrequency    = SAI_AUDIO_FREQUENCY_48K; // _48K or _96K or _192K
    hsai_a.Init.Protocol          = SAI_FREE_PROTOCOL;
    hsai_a.Init.DataSize          = SAI_DATASIZE_16;
    hsai_a.Init.FirstBit          = SAI_FIRSTBIT_MSB;
    hsai_a.Init.ClockStrobing     = SAI_CLOCKSTROBING_RISINGEDGE; // CONFIRM

    hsai_a.FrameInit.FrameLength          = 17; // was 33 for 32 vals
    hsai_a.FrameInit.ActiveFrameLength    = 1; // 1 ?
    hsai_a.FrameInit.FSDefinition         = SAI_FS_STARTFRAME;
    hsai_a.FrameInit.FSPolarity           = SAI_FS_ACTIVE_HIGH;
    hsai_a.FrameInit.FSOffset             = SAI_FS_BEFOREFIRSTBIT;

    hsai_a.SlotInit.FirstBitOffset    = 0;
    hsai_a.SlotInit.SlotSize          = SAI_SLOTSIZE_16B;
    hsai_a.SlotInit.SlotNumber        = 8;
    hsai_a.SlotInit.SlotActive        = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1
                                      | SAI_SLOTACTIVE_2 | SAI_SLOTACTIVE_3
                                      | SAI_SLOTACTIVE_4 | SAI_SLOTACTIVE_5
                                      | SAI_SLOTACTIVE_6 | SAI_SLOTACTIVE_7;

    if(HAL_SAI_Init(&hsai_a)){
        printf("sai init failed\n\r");
        return;
    }

    // Enable SAI to generate clock used by audio driver
    __HAL_SAI_ENABLE(&hsai_a);
}

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
    GPIO_InitTypeDef  GPIO_Init;
    GPIO_Init.Mode  = GPIO_MODE_AF_PP;
    GPIO_Init.Pull  = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_SAI1_CLK_ENABLE(); // RCC
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_Init.Alternate     = GPIO_AF6_SAI1;
    GPIO_Init.Pin           = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOE, &GPIO_Init);
    GPIO_Init.Alternate     = GPIO_AF6_SAI1;
    GPIO_Init.Pin           = GPIO_PIN_5;
    HAL_GPIO_Init(GPIOE, &GPIO_Init);
    GPIO_Init.Alternate     = GPIO_AF6_SAI1;
    GPIO_Init.Pin           = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOE, &GPIO_Init);

    // Configure DMA used for SAI1_A
    // d2.s3.c0 // alternates: 2.1.0, 2.3.0, 2.6.10
    hdma_tx_a.Init.Channel                = DMA_CHANNEL_0;
    hdma_tx_a.Init.Direction              = DMA_MEMORY_TO_PERIPH;
    hdma_tx_a.Init.PeriphInc              = DMA_PINC_DISABLE;
    hdma_tx_a.Init.MemInc                 = DMA_MINC_ENABLE;
    hdma_tx_a.Init.PeriphDataAlignment    = DMA_PDATAALIGN_HALFWORD;
    hdma_tx_a.Init.MemDataAlignment       = DMA_MDATAALIGN_HALFWORD;
    hdma_tx_a.Init.Mode                   = DMA_CIRCULAR;
    hdma_tx_a.Init.Priority               = DMA_PRIORITY_HIGH;
    hdma_tx_a.Init.FIFOMode               = DMA_FIFOMODE_DISABLE;
    hdma_tx_a.Init.FIFOThreshold          = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_tx_a.Init.MemBurst               = DMA_MBURST_SINGLE;
    hdma_tx_a.Init.PeriphBurst            = DMA_PBURST_SINGLE;

    hdma_tx_a.Instance                    = DMA2_Stream3;

    // Bidirectionally link the DMA & SAI handles
    __HAL_LINKDMA(hsai, hdmatx, hdma_tx_a);

    // Deinitialize the Stream for new transfer
    HAL_DMA_DeInit(&hdma_tx_a);

    // Configure the DMA Stream
    if( HAL_OK != HAL_DMA_Init(&hdma_tx_a) ){
        // debug(&debug, "HAL_DMA_Init failed");
        return;
    }

    // Codec request triggers transfer & new frame calc
    HAL_NVIC_SetPriority( DMA2_Stream3_IRQn
                        , DAC_IRQPriority
                        , 1
                        );
    HAL_NVIC_EnableIRQ( DMA2_Stream3_IRQn );
}

void sai_start_transmit(uint16_t* hwords, uint16_t count){
    if(HAL_SAI_Transmit_DMA(&hsai_a, (uint8_t*)hwords, count)){
        printf("sai transmit fail.\n\r");
    }
}



// DMA triggered by codec requesting more ADC!
void DMA2_Stream3_IRQHandler(void){
    HAL_DMA_IRQHandler(&hdma_tx_a);
    if(hdma_tx_a.ErrorCode != HAL_OK){
        printf("sai dma err: 0x%0x\n\r", hdma_tx_a.ErrorCode);
    }
}

static void callback(int offset){
    // HAL_NVIC_DisableIRQ( DMA2_Stream3_IRQn );
    // int b_size2 = b_size * 2;
    // int offset2 = offset * b_size2;
    // float in[b_size2];
    // float out[b_size2];
    // codec_to_floats( in, &in[b_size], &inBuff[offset2], b_size );
// FIXME ONLY ONE OF THE NEXT 2 FUNS

    ADDA_BlockProcess( samples );





    // (*block_process_fn)( out
    //                    , in // treat ins as zeroes (unused)
    //                    , b_size
    //                    );
    // floats_to_codec( &outBuff[offset2], out, b_size );
    // HAL_NVIC_EnableIRQ( DMA2_Stream3_IRQn );
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){ callback(0); }
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){ callback(1); }
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai){
    printf("sai error 0x%x\n\r", hsai->ErrorCode);
}

