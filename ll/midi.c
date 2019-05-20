#include "midi.h"

UART_HandleTypeDef midiuart;

uint8_t rx_buf[64];

// private declarations
static uint8_t MIDI_rx_cmd( void );
static uint8_t MIDI_rx_data( uint8_t count );
void MIDI_event( void );

// public defns
void MIDI_Init( void )
{
	midiuart.Instance = MIDIx;

	midiuart.Init.BaudRate     = MIDI_baud;
	midiuart.Init.WordLength   = UART_WORDLENGTH_8B;
	midiuart.Init.StopBits     = UART_STOPBITS_1;
	midiuart.Init.Parity       = UART_PARITY_NONE;
	midiuart.Init.Mode         = UART_MODE_RX;
	midiuart.Init.OverSampling = UART_OVERSAMPLING_16; // 16 or 8. d=16
    if( HAL_UART_Init( &midiuart ) ){ printf("!midi_init\n"); }

    while( HAL_UART_GetState( &midiuart ) != HAL_UART_STATE_READY ){}

    if( MIDI_rx_cmd() ){ ;; } // error starting midi propogate to main?
}

void MIDI_DeInit(void)
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

	hdma_rx.Instance				= MIDIx_RX_DMA_STREAM;
	hdma_rx.Init.Channel			= MIDIx_RX_DMA_CHANNEL;
	hdma_rx.Init.Direction			= DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc			= DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc				= DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment= DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment	= DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode 				= DMA_NORMAL;
	hdma_rx.Init.Priority 			= DMA_PRIORITY_HIGH;
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

void MIDIx_DMA_RX_IRQHandler( void )
{
	HAL_DMA_IRQHandler( midiuart.hdmarx );
}

uint8_t receiving_packet = 0;
static uint8_t MIDI_rx_cmd( void )
{
    receiving_packet = 0;
    uint8_t err;
    BLOCK_IRQS(
        err = HAL_UART_Receive_DMA( &midiuart
                                  , &(rx_buf[0])
                                  , 1
                                  );
    );
    if( err ){ printf("midi_cmd_error\n"); }
    return err;
}

static uint8_t MIDI_rx_data( uint8_t count )
{
    receiving_packet = 1;
    uint8_t err;
    BLOCK_IRQS(
        err = HAL_UART_Receive_DMA( &midiuart
                                  , &(rx_buf[1])
                                  , count
                                  );
    );
    if( err ){ printf("midi_data_error\n"); }
    return err;
}

typedef enum{ MIDI_NOTEOFF       = 0x80
            , MIDI_NOTEON        = 0x90
            , MIDI_AFTERTOUCH    = 0xA0
            , MIDI_CC            = 0xB0
            , MIDI_PATCHCHANGE   = 0xC0
            , MIDI_CH_PRESSURE   = 0xD0
            , MIDI_PITCHBEND     = 0xE0
            , MIDI_SYSEX         = 0xF0
            , MIDI_MTC_QUARTER   = 0xF1
            , MIDI_SONG_POS      = 0xF2
            , MIDI_SONG_SEL      = 0xF3
            , MIDI_TUNE          = 0xF6
            , MIDI_SYSEX_END     = 0xF7
            , MIDI_CLOCK         = 0xF8
            , MIDI_START         = 0xFA
            , MIDI_CONTINUE      = 0xFB
            , MIDI_STOP          = 0xFC
            , MIDI_ACTIVE_SENSE  = 0xFE
            , MIDI_SYS_RESET     = 0xFF

} MIDI_CMD_t;

uint8_t sysex_count = 0;

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *huart )
{
    if( !receiving_packet ){ // this is a new midi message
        switch( rx_buf[0] & 0xF0 ){ // ignore channel data
            case MIDI_NOTEOFF:     MIDI_rx_data( 2 ); break;
            case MIDI_NOTEON:      MIDI_rx_data( 2 ); break;
            case MIDI_AFTERTOUCH:  MIDI_rx_data( 2 ); break;
            case MIDI_CC:          MIDI_rx_data( 2 ); break;
            case MIDI_PATCHCHANGE: MIDI_rx_data( 2 ); break;
            case MIDI_CH_PRESSURE: MIDI_rx_data( 1 ); break;
            case MIDI_PITCHBEND:   MIDI_rx_data( 2 ); break;
            case MIDI_SYSEX:
                if( rx_buf[0] != MIDI_SYSEX ){ MIDI_event(); }
                else {
                    // TODO begin sysex packet (need to set metadata)
                    sysex_count = 1;
                    MIDI_rx_data( 1 );
                }
                break;
            default: MIDI_rx_cmd(); break; // retry
        }
    } else { // receiving data
        if( (rx_buf[0] & 0xF0) == MIDI_SYSEX ){
            if( rx_buf[sysex_count] == MIDI_SYSEX_END ){
                MIDI_event();
            } else {
                sysex_count++; // save data
                MIDI_rx_data( 1 ); // get next byte
            }
        } else {
            if( rx_buf[0] == MIDI_NOTEON ){
                printf("noteon %i %i %i\n", rx_buf[0], rx_buf[1], rx_buf[2]);
            }
            MIDI_event(); }
    }
}

void MIDI_event( void )
{
    // TODO lua callback
    switch( rx_buf[0] & 0xF0 ){ // ignore channel data while switching
        case MIDI_NOTEOFF:      break;
        case MIDI_NOTEON:       break;
        case MIDI_AFTERTOUCH:   break;
        case MIDI_CC:           break;
        case MIDI_PATCHCHANGE:  break;
        case MIDI_CH_PRESSURE:  break;
        case MIDI_PITCHBEND:    break;
        case MIDI_SYSEX:
            switch( rx_buf[0] ){
                case MIDI_SYSEX:
                    // use sysex_count in handler
                    sysex_count = 0;
                    break;
                case MIDI_MTC_QUARTER:   break;
                case MIDI_SONG_POS:      break;
                case MIDI_SONG_SEL:      break;
                case MIDI_TUNE:          break;
                case MIDI_CLOCK:         break;
                case MIDI_START:         break;
                case MIDI_CONTINUE:      break;
                case MIDI_STOP:          break;
                case MIDI_ACTIVE_SENSE:  break;
                case MIDI_SYS_RESET:     break;
                default: break;
            }
        default: break;
    }
    MIDI_rx_cmd(); // restart reception!
}

void MIDIx_IRQHandler( void )
{
	HAL_UART_IRQHandler( &midiuart );
}
