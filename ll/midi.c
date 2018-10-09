#include "midi.h"

UART_HandleTypeDef midiuart;

uint8_t rx_buf[64];

void MIDI_Init( void )
{
	midiuart.Instance = MIDIx;

	midiuart.Init.BaudRate     = MIDI_baud;
	midiuart.Init.WordLength   = UART_WORDLENGTH_8B;
	midiuart.Init.StopBits     = UART_STOPBITS_1;
	midiuart.Init.Parity       = UART_PARITY_NONE;
	midiuart.Init.Mode         = UART_MODE_RX; // should just be _RX
	midiuart.Init.OverSampling = UART_OVERSAMPLING_16; // 16 or 8. d=16
    if(HAL_UART_Init( &midiuart ) ){
        U_PrintLn("!midi_init");
    }

    while( HAL_UART_GetState( &midiuart ) != HAL_UART_STATE_READY ){}
    if(HAL_UART_Receive_DMA( &midiuart
                           , rx_buf
                           , 1
    				       )){ U_PrintLn("midi_error"); }
    //uint8_t errorcode;
    //if((errorcode = HAL_UART_Receive( &midiuart
    //                     , rx_buf
    //                     , 1
    //                     , 2000
    //				     ))){ U_Print("!midi_"); U_PrintU8(errorcode); }
}

void MIDI_DeInit(void)
{
	HAL_UART_DeInit( &midiuart );
}


// LOW LEVEL UART HARDWARE CONFIGURATION FUNCTION
void HAL_UART_MspInit(UART_HandleTypeDef *hu )
//void MIDI_MspInit(UART_HandleTypeDef *hu )
{
    //if( hu != &midiuart ){ U_PrintLn("bad match"); }

    MIDIx_FORCE_RESET();
    MIDIx_RELEASE_RESET();

	static DMA_HandleTypeDef hdma_rx;
    U_PrintLn("midi setup");

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

	// Configure DMA
	hdma_rx.Instance				= MIDIx_RX_DMA_STREAM;
	hdma_rx.Init.Channel			= MIDIx_RX_DMA_CHANNEL;
	hdma_rx.Init.Direction			= DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc			= DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc				= DMA_MINC_DISABLE;
	hdma_rx.Init.PeriphDataAlignment= DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment	= DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode 				= DMA_NORMAL;
	hdma_rx.Init.Priority 			= DMA_PRIORITY_HIGH;
    if( HAL_DMA_Init( &hdma_rx ) ){ U_PrintLn("!MIDI_DMA"); }
	__HAL_LINKDMA( hu, hdmarx, hdma_rx ); // Associate DMA to UART handle

    // Configure NVIC for DMA
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

void MIDIx_DMA_RX_IRQHandler( void )
{
	HAL_DMA_IRQHandler( midiuart.hdmarx );
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart )
{
    if(HAL_UART_Receive_DMA( huart
                         , rx_buf
                         , 1
					     )){ U_PrintLn("midi_error"); }
    U_PrintU8(rx_buf[0]);
}

void MIDIx_IRQHandler( void )
{
    U_PrintLn("midiIRQ");
	HAL_UART_IRQHandler( &midiuart );
}
