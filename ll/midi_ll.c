#include "midi_ll.h"

UART_HandleTypeDef midiuart;

void rx_callback_null( uint8_t* buf );
void error_callback_null( void );
void (*rx_callback)(uint8_t*) = rx_callback_null;
uint8_t rx_buf[8]; // TODO not big enough for sysex!
void (*error_callback)(void) = error_callback_null;

// public defns
void MIDI_ll_Init( void(*rx_handler)(uint8_t*)
                 , void(*error_handler)(void) )
{
    midiuart.Instance = MIDIx;

    midiuart.Init.BaudRate     = MIDI_baud;
    midiuart.Init.WordLength   = UART_WORDLENGTH_8B;
    midiuart.Init.StopBits     = UART_STOPBITS_1;
    midiuart.Init.Parity       = UART_PARITY_NONE;
    midiuart.Init.Mode         = UART_MODE_RX;
    midiuart.Init.OverSampling = UART_OVERSAMPLING_16; // 16 or 8. d=16
    if( HAL_UART_Init( &midiuart ) ){ printf("!midi_uart_init\n"); }

    while( HAL_UART_GetState( &midiuart ) != HAL_UART_STATE_READY ){}

    rx_callback = rx_handler;
    error_callback = error_handler;
}

void MIDI_ll_DeInit(void)
{
    HAL_UART_DeInit( &midiuart );
}

void HAL_UART_MspInit(UART_HandleTypeDef *hu )
{
    MIDIx_FORCE_RESET();
    MIDIx_RELEASE_RESET();

    static DMA_HandleTypeDef hdma_rx;

    MIDI_UART_RCC();
    MIDI_GPIO_RCC();
    MIDI_DMA_CLK_ENABLE();

    GPIO_InitTypeDef gpio;
    gpio.Pin       = MIDI_RXPIN;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLUP;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = MIDI_AF;
    HAL_GPIO_Init( MIDI_GPIO, &gpio );

    hdma_rx.Instance                 = MIDIx_RX_DMA_STREAM;
    hdma_rx.Init.Channel             = MIDIx_RX_DMA_CHANNEL;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    if( HAL_DMA_Init( &hdma_rx ) ){ printf("!MIDI_DMA\n"); }
    __HAL_LINKDMA( hu, hdmarx, hdma_rx ); // Associate DMA to UART handle

    HAL_NVIC_SetPriority( MIDIx_DMA_RX_IRQn
                        , MIDIx_DMA_IRQPriority
                        , MIDIx_DMA_IRQSubPriority
                        );
    HAL_NVIC_EnableIRQ( MIDIx_DMA_RX_IRQn );

    HAL_NVIC_SetPriority( MIDIx_IRQn
                        , MIDIx_IRQPriority
                        , MIDIx_IRQSubPriority
                        );
    HAL_NVIC_EnableIRQ( MIDIx_IRQn );
}

void MIDI_ll_DMA_RX_IRQHandler( void )
{
    HAL_DMA_IRQHandler( midiuart.hdmarx );
}

int MIDI_ll_Rx( int ix, int count )
{
    int err;
    err = HAL_UART_Receive_DMA( &midiuart
                              , &(rx_buf[ix])
                              , count
                              );
    if( err ){ printf("MIDI_ll_Rx %i\n", err); }
    return err;
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart )
{
    (*rx_callback)(rx_buf);
}
void HAL_UART_ErrorCallback( UART_HandleTypeDef *huart )
{
    printf("uart_error: %i\n", huart->ErrorCode);
    (*error_callback)();
}

void MIDI_ll_IRQHandler( void )
{
    HAL_UART_IRQHandler( &midiuart );
}

void rx_callback_null( uint8_t* buf ){}
void error_callback_null( void ){}
