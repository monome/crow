#include "dac108.h"

#include <stdio.h>
#include "stdlib.h" // malloc()
#include "adda.h"

#define DAC_BUFFER_COUNT 2 // ping-pong

// pointer to the malloc()d buffer from DAC_Init()
static uint16_t  samp_count = 0;
static uint32_t* samples = NULL;
static int b_size = 0;

static const uint16_t write_through_mode = 0b1001000000000000;

#define _DMA_PDATA_ALIGN DMA_PDATAALIGN_WORD
#define _DMA_MDATA_ALIGN DMA_MDATAALIGN_WORD
#define _DMA_FIFO_THRESH DMA_FIFO_THRESHOLD_HALFFULL
#define _T_SAMPLES uint32_t*
#define _SAI_DATASIZE SAI_DATASIZE_16
#define _SAI_FIRSTBIT SAI_FIRSTBIT_MSB
#define _SAI_CLOCKSTROBE SAI_CLOCKSTROBING_RISINGEDGE

#define _FRAME_LENGTH ((16*2)+1)
#define _FRAME_ACTIVE_LENGTH 1
#define _FRAME_FSDEFINTION SAI_FS_STARTFRAME
#define _FRAME_FSPOLARITY SAI_FS_ACTIVE_HIGH
#define _FRAME_FSOFFSET SAI_FS_BEFOREFIRSTBIT

#define _SLOT_FIRSTBITOFFSET 0
#define _SLOT_SIZE SAI_SLOTSIZE_16B
#define _SLOT_NUMBER 2
#define _SLOT_ACTIVE (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1)

static void sai_start_transmit(uint32_t* hwords, uint16_t count);
static void sai_init(void);

void DAC_Init(uint16_t bsize, uint8_t chan_count){
    b_size = bsize;
    sai_init();

    // Create the sample buffer for DMA transfer
    samp_count = DAC_BUFFER_COUNT * bsize * chan_count; // 512
    printf("samp_count %x\n\r", samp_count);
    samples = malloc( sizeof(uint32_t) * samp_count ); // 1k
    if(samples == NULL){ printf("!DAC_buffer\n"); }
    // for( int i=0; i<samp_count; i++ ){ samples[i] = 0; } // unnecessary
}

void DAC_Start(void){
    for(int i=0; i<samp_count; i++){ // double-buffered
        // NB: we add 1 to i, so we start the loop on second channel
        // int chan_mod = (i+1) & (8-1); // channel modulo 8, running 1-7, then 0.
        // if(chan_mod == 0){ // chan 0
        //     samples[i] = 0b1011 << 12; // sets chan 1 & updates all outputs
        // } else { // chans 1-7
        //     samples[i] = chan_mod<<12; // sets channel select bits
        // }
        samples[i] = write_through_mode;
    }
    // sai_start_transmit(samples, samp_count>>1);
    sai_start_transmit(samples, samp_count); // in terms of bytes?
}

static const int channel_id_from_ix[12] = {
    DAC_Fold1,
    DAC_Fold2,
    DAC_Fold3,
    DAC_Fold4,
    DAC_Fold5,
    DAC_Fold6,
    DAC_Fold7,
    DAC_Fold8,
    DAC_Fold9,
    DAC_Fold10,
    DAC_Fold11,
    DAC_Fold12
};

int DAC_get_channel_id(int channel){
    return channel_id_from_ix[channel];
}

void DAC_CalibrateScalar( uint8_t channel, float scale ){
    // dac_calibrated_scalar[channel] = DAC_V_TO_U16 * scale;
}

void DAC_CalibrateOffset( uint8_t channel, float volts ){
    // dac_calibrated_offset[channel] = volts;
}

uint16_t lim_i32_u12( int32_t v ){
    return (uint16_t)((v > 0xfff) ? 0xfff : (v < 0) ? 0 : v);
}

/* Does all the work converting a generic representation into serial packets
 * Convert floats (representing volts) to u16 representation
 * Interleave a block of each channel into a stream
 * */
// unpickled_data is arranged as blocks of bsize, * chan count
// pickled data needs to be interleaved sample-by-sample across all channels
void DAC_PickleBlock( uint32_t* dac_pickle_ptr
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
    // for( int j=0; j<ADDA_DAC_CHAN_COUNT; j++ ){
    //     mul_vf_f( &(unpickled_data[j*bsize])
    //             , dac_calibrated_scalar[j] // scale volts up to u12. 409.5 per octave
    //             , bsize
    //             );
    // }

// TESTING: just create a stair step of outputs
    /*
    for( int i=0; i<bsize; i++){
        for( int j=0; j<ADDA_DAC_CHAN_COUNT; j++ ){
            if((i==0) && (j==0 | j==1)){
                // dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] = 0b11111111;
                dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] = 0;
            } else if((i==1) && (j==0)){
                // dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] = 0b10000001;
                dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] = 0b1000000000000001;
            } else {
                dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] = 0;
            }
            // dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] = 1 << j; // add channel tag
            // dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] &= 0xfff; // clip any overage

            // int b_chan = i % ADDA_DAC_CHAN_COUNT;
            // dac_pickle_ptr[j+i*ADDA_DAC_CHAN_COUNT] |= b_chan << 12; // add channel tag
        }
    }

*/

    // +/-5v lfo -> +/-2048.f
    // uint16_t vv = lim_i32_u12((int32_t)(DAC_ZERO_VOLTS - unpickled_data[0]));
    // vv |= (7<<12);
    // uint16_t* dpp = (uint16_t*)dac_pickle_ptr;


    /*
    for(int i=0; i<(ADDA_DAC_CHAN_COUNT * bsize); i++){
        // 0chan + 0
        // 1chan + 0
        // ...
        // 0chan + 1
        // ? = weird scan through the buffer to interleave channels
        int b_chan = i % ADDA_DAC_CHAN_COUNT;
        int b_samp = i / ADDA_DAC_CHAN_COUNT;
        // DAC_ZERO_VOLTS = 0x7ff (2047)
        // dac_pickle_ptr[i] = vv | (b_chan << 12);
        // dac_pickle_ptr[i] = vv;
        dac_pickle_ptr[i] = lim_i32_u12((int32_t)(DAC_ZERO_VOLTS - unpickled_data[b_samp + bsize*b_chan]));
        dac_pickle_ptr[i] |= b_chan << 12;
        // dac_pickle_ptr[i] <<= 16;
        // *dpp = lim_i32_u12((int32_t)(DAC_ZERO_VOLTS - unpickled_data[b_samp + bsize*b_chan]));
        // *dpp |= b_chan << 12;
        // dpp++;
    }
    */
}

static SAI_HandleTypeDef hsai_a;
static DMA_HandleTypeDef hdma_tx_a;

static void sai_init(void){
    RCC_PeriphCLKInitTypeDef rcc;
    rcc.PeriphClockSelection    = RCC_PERIPHCLK_SAI2;
    rcc.Sai2ClockSelection      = RCC_SAI2CLKSOURCE_PLLSAI;

    // 30MHz maximum bit clock in daisy-chain mode
    // every 2 channels takes 33 clks
    // for 16 channels, that's 528 clks
    // at 30MHz (max) that's 56,818Hz
    // 48kHz -> 25,344,000 core clock


    // here we configure for 3.072MHz
    // ie 8 channels, 17bits, 22.5kHz sample rate
    // rcc.PLLSAI.PLLSAIN          = 384;
    // rcc.PLLSAI.PLLSAIN          = 192;

    // 6kHz
    rcc.PLLSAI.PLLSAIN          = 396;
    rcc.PLLSAI.PLLSAIQ          = 5;
    rcc.PLLSAIDivQ              = 25;

    // 24kHz
    // rcc.PLLSAI.PLLSAIN          = 431;
    // rcc.PLLSAI.PLLSAIQ          = 2;
    // rcc.PLLSAIDivQ              = 17;

    // 48kHz
    // rcc.PLLSAI.PLLSAIN          = 380;
    // rcc.PLLSAI.PLLSAIQ          = 3;
    // rcc.PLLSAIDivQ              = 5;

    // rcc.PLLSAI.PLLSAIN          = 96;
    // rcc.PLLSAI.PLLSAIQ          = 5;
    // rcc.PLLSAIDivQ              = 25;
    // see @ciel/tools/sai_pll_calculator.lua to configure

    HAL_RCCEx_PeriphCLKConfig(&rcc);

    // Initialize SAI
    __HAL_SAI_RESET_HANDLE_STATE(&hsai_a);

    // block A
    hsai_a.Instance = SAI2_Block_B; // TODO follow instance
    __HAL_SAI_DISABLE(&hsai_a);
    hsai_a.Init.AudioMode         = SAI_MODEMASTER_TX;
    hsai_a.Init.Synchro           = SAI_ASYNCHRONOUS;
    hsai_a.Init.SynchroExt        = SAI_SYNCEXT_DISABLE;
    hsai_a.Init.OutputDrive       = SAI_OUTPUTDRIVE_ENABLE;
    hsai_a.Init.NoDivider         = SAI_MASTERDIVIDER_DISABLE;
    hsai_a.Init.FIFOThreshold     = SAI_FIFOTHRESHOLD_1QF;
    hsai_a.Init.AudioFrequency    = SAI_AUDIO_FREQUENCY_48K; // _48K or _96K or _192K
    hsai_a.Init.Protocol          = SAI_FREE_PROTOCOL;
    hsai_a.Init.DataSize          = _SAI_DATASIZE;
    hsai_a.Init.FirstBit          = _SAI_FIRSTBIT;
    hsai_a.Init.ClockStrobing     = _SAI_CLOCKSTROBE; // CONFIRM

    hsai_a.FrameInit.FrameLength          = _FRAME_LENGTH; // ie data length plus 1 bit for FS sync pulse
    hsai_a.FrameInit.ActiveFrameLength    = _FRAME_ACTIVE_LENGTH;
    hsai_a.FrameInit.FSDefinition         = _FRAME_FSDEFINTION;
    hsai_a.FrameInit.FSPolarity           = _FRAME_FSPOLARITY;
    hsai_a.FrameInit.FSOffset             = _FRAME_FSOFFSET; // SAI_FS_FIRSTBIT

    hsai_a.SlotInit.FirstBitOffset    = _SLOT_FIRSTBITOFFSET; // maybe 1?
    hsai_a.SlotInit.SlotSize          = _SLOT_SIZE;
    hsai_a.SlotInit.SlotNumber        = _SLOT_NUMBER; // ie. only 1 chip in sequence
    hsai_a.SlotInit.SlotActive        = _SLOT_ACTIVE; // each DAC chan needs it's own frame!

    if(HAL_SAI_Init(&hsai_a)){
        printf("sai init failed\n\r");
        return;
    }

    // Enable SAI to generate clock used by audio driver
    __HAL_SAI_ENABLE(&hsai_a);
}

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai){
    GPIO_InitTypeDef  GPIO_Init;
    GPIO_Init.Mode  = GPIO_MODE_AF_PP;
    GPIO_Init.Pull  = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_SAI2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_Init.Alternate     = GPIO_AF10_SAI2;
    GPIO_Init.Pin           = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOA, &GPIO_Init);
    GPIO_Init.Alternate     = GPIO_AF8_SAI2;
    GPIO_Init.Pin           = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOA, &GPIO_Init);
    GPIO_Init.Alternate     = GPIO_AF8_SAI2;
    GPIO_Init.Pin           = GPIO_PIN_0;
    HAL_GPIO_Init(GPIOC, &GPIO_Init);

    // Configure DMA used for SAI2_B
    // d2.s6.c3 // alternates: 2.7.0
    hdma_tx_a.Init.Channel                = DMA_CHANNEL_3;
    hdma_tx_a.Init.Direction              = DMA_MEMORY_TO_PERIPH;
    hdma_tx_a.Init.PeriphInc              = DMA_PINC_DISABLE;
    hdma_tx_a.Init.MemInc                 = DMA_MINC_ENABLE;
    hdma_tx_a.Init.PeriphDataAlignment    = _DMA_PDATA_ALIGN;
    // hdma_tx_a.Init.MemDataAlignment       = DMA_MDATAALIGN_HALFWORD;
    hdma_tx_a.Init.MemDataAlignment       = _DMA_MDATA_ALIGN;
    hdma_tx_a.Init.Mode                   = DMA_CIRCULAR;
    hdma_tx_a.Init.Priority               = DMA_PRIORITY_HIGH;
    hdma_tx_a.Init.FIFOMode               = DMA_FIFOMODE_DISABLE;
    hdma_tx_a.Init.FIFOThreshold          = _DMA_FIFO_THRESH;
    hdma_tx_a.Init.MemBurst               = DMA_MBURST_SINGLE;
    hdma_tx_a.Init.PeriphBurst            = DMA_PBURST_SINGLE;

    hdma_tx_a.Instance                    = DMA2_Stream6;

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
    HAL_NVIC_SetPriority( DMA2_Stream6_IRQn
                        , DAC_IRQPriority
                        , 1
                        );
    HAL_NVIC_EnableIRQ( DMA2_Stream6_IRQn );
}

void sai_start_transmit(_T_SAMPLES hwords, uint16_t count){
    if(HAL_SAI_Transmit_DMA(&hsai_a, (uint8_t*)hwords, count)){
        printf("sai transmit fail.\n\r");
    }
}



// static int status = 0;
// DMA triggered by codec requesting more ADC!
void DMA2_Stream6_IRQHandler(void){
    // status ^= 1;
    // TP_debug_led(0, status);
    HAL_DMA_IRQHandler(&hdma_tx_a);
    if(hdma_tx_a.ErrorCode != HAL_OK){
        // printf("sai dma err: 0x%0x\n\r", hdma_tx_a.ErrorCode);
        printf("sai dma err\n\r");
    }
}

static inline void callback(int offset){
    ADDA_BlockProcess(&samples[offset], b_size);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){ callback(0); }
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){ callback(samp_count>>1); }
// void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){ callback(samp_count>>1); }
// void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){ callback(0); }
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai){
    // printf("sai error 0x%x\n\r", hsai->ErrorCode);
    printf("sai error");
}
