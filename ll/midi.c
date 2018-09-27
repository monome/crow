#include "midi.h"

USART_HandleTypeDef midiusart;

uint8_t rx_buf[64];

void MIDI_Init( void )
{
	midiusart.Instance = MIDIx;

	midiusart.Init.BaudRate   = MIDI_baud;
	midiusart.Init.WordLength = USART_WORDLENGTH_8B;
	midiusart.Init.StopBits   = USART_STOPBITS_1;
	midiusart.Init.Parity     = USART_PARITY_NONE;
	midiusart.Init.Mode       = USART_MODE_RX;
	HAL_USART_Init( &midiusart );

    while( HAL_USART_GetState( &midiusart ) != HAL_USART_STATE_READY ){}
    if(HAL_USART_Receive_DMA( &midiusart
                         , rx_buf
                         , 1
    				     )){ U_PrintLn("midi_error"); }
}

void MIDI_DeInit(void)
{
	HAL_USART_DeInit( &midiusart );
}


// LOW LEVEL USART HARDWARE CONFIGURATION FUNCTION
void MIDI_MspInit(USART_HandleTypeDef *hu )
{
	static DMA_HandleTypeDef hdma_rx;

	MIDI_USART_RCC();
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
	hdma_rx.Init.MemInc				= DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment= DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment	= DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode 				= DMA_NORMAL;
	hdma_rx.Init.Priority 			= DMA_PRIORITY_HIGH;
	HAL_DMA_Init( &hdma_rx );
	__HAL_LINKDMA( hu, hdmarx, hdma_rx ); // Associate DMA to USART handle

	// Configure NVIC for DMA
	HAL_NVIC_SetPriority( MIDIx_DMA_RX_IRQn
                        , MIDIx_DMA_IRQPriority
                        , MIDIx_DMA_IRQSubPriority
                        );
	HAL_NVIC_EnableIRQ( MIDIx_DMA_RX_IRQn );

	HAL_NVIC_SetPriority( MIDIx_IRQn
                        , MIDIx_IRQSubPriority
                        , MIDIx_IRQSubPriority
                        );
	HAL_NVIC_EnableIRQ( MIDIx_IRQn );
}

void MIDIx_DMA_RX_IRQHandler( void )
{
    U_PrintLn(" ## ");
	HAL_DMA_IRQHandler( midiusart.hdmarx );
}

void HAL_USART_RxCpltCallback( USART_HandleTypeDef *husart )
{
    U_PrintLn("midi_rx");
    if(HAL_USART_Receive_DMA( &midiusart
                         , rx_buf
                         , 1
					     )){ U_PrintLn("midi_error"); }
}

void MIDIx_IRQHandler( void )
{
    U_PrintLn("midiIRQ");
	HAL_USART_IRQHandler( &midiusart );
}
