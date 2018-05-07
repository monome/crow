#include "debug_usart.h"

#include <string.h>

#include "str_buffer.h"

USART_HandleTypeDef handusart;
str_buffer_t str_buf;

#ifdef RELEASE
void Debug_USART_Init(void){ return; }
void U_PrintNow(void){ return; }
void U_Print(char* s){ return; }
void U_PrintU32(uint32_t n){ return; }
void U_PrintU16(uint32_t n){ return; }
void U_PrintU8(uint8_t n){ return; }
void U_PrintVar(char* name, uint32_t n, uint8_t ret_flag){ return; }
#endif // RELEASE

#ifdef DEBUG
void Debug_USART_Init( void )
{
	handusart.Instance = DBG_USARTx;

	handusart.Init.BaudRate   = DBG_USART_baud;
	handusart.Init.WordLength = USART_WORDLENGTH_8B;
	handusart.Init.StopBits   = USART_STOPBITS_1;
	handusart.Init.Parity     = USART_PARITY_NONE;
	handusart.Init.Mode       = USART_MODE_TX_RX;
	HAL_USART_Init( &handusart );

	str_buffer_init( &str_buf, 512 ); // fifo for DMA buffer
}

void Debug_USART_DeInit(void)
{
	HAL_USART_DeInit( &handusart );
	str_buffer_deinit( &str_buf );
}


// LOW LEVEL USART HARDWARE CONFIGURATION FUNCTION
void HAL_USART_MspInit(USART_HandleTypeDef *hu )
{
	static DMA_HandleTypeDef hdma_tx;
	// static DMA_HandleTypeDef hdma_rx;

	DBG_USART_USART_RCC();
	DBG_USART_GPIO_RCC();
	DBG_DMAx_CLK_ENABLE();

	GPIO_InitTypeDef gpio;
	gpio.Pin       = DBG_USART_TXPIN;
	gpio.Mode      = GPIO_MODE_AF_PP;
	gpio.Pull      = GPIO_PULLUP;
	gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = DBG_USART_AF;
	HAL_GPIO_Init( DBG_USART_GPIO, &gpio );

	gpio.Pin = DBG_USART_RXPIN;
	HAL_GPIO_Init( DBG_USART_GPIO, &gpio );

	// Configure DMA
	hdma_tx.Instance                 = USARTx_TX_DMA_STREAM;
	hdma_tx.Init.Channel             = USARTx_TX_DMA_CHANNEL;
	hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_tx.Init.Mode                = DMA_NORMAL;
	hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
	HAL_DMA_Init( &hdma_tx );
	__HAL_LINKDMA( hu, hdmatx, hdma_tx ); // Associate DMA to USART handle

/*	hdma_rx.Instance				= USARTx_RX_DMA_STREAM;
	hdma_rx.Init.Channel			= USARTx_RX_DMA_CHANNEL;
	hdma_rx.Init.Direction			= DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc			= DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc				= DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment= DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment	= DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode 				= DMA_NORMAL;
	hdma_rx.Init.Priority 			= DMA_PRIORITY_HIGH;
	HAL_DMA_Init( &hdma_rx );
	__HAL_LINKDMA( hu, hdmarx, hdma_rx ); // Associate DMA to USART handle
*/
	// Configure NVIC for DMA
	HAL_NVIC_SetPriority( USARTx_DMA_TX_IRQn, 5, 1 );
	HAL_NVIC_EnableIRQ(   USARTx_DMA_TX_IRQn);

	// HAL_NVIC_SetPriority(USARTx_DMA_RX_IRQn, 5, 2);
	// HAL_NVIC_EnableIRQ(USARTx_DMA_RX_IRQn);

	HAL_NVIC_SetPriority( USARTx_IRQn, 5, 0 );
	HAL_NVIC_EnableIRQ(   USARTx_IRQn);
}

void USARTx_DMA_RX_IRQHandler( void )
{
	HAL_DMA_IRQHandler( handusart.hdmarx );
}

void HAL_USART_RxCpltCallback( USART_HandleTypeDef *husart )
{
}

void USARTx_DMA_TX_IRQHandler( void )
{
	HAL_DMA_IRQHandler( handusart.hdmatx );
}

void HAL_USART_TxCpltCallback( USART_HandleTypeDef *husart )
{
    U_PrintNow();
}

void USARTx_IRQHandler( void )
{
	HAL_USART_IRQHandler( &handusart );
}

// Communication Functions

void U_PrintNow( void )
{
	if( HAL_USART_GetState( &handusart ) == HAL_USART_STATE_READY
	 && !str_buffer_empty( &str_buf ) ){
// mask interrupts to ensure transmission!
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
	    uint16_t str_len = str_buffer_len( &str_buf );
	    HAL_USART_Transmit_DMA( &handusart
	                          , (uint8_t*)str_buffer_dequeue( &str_buf, str_len )
                              , str_len
						      );
__set_PRIMASK( old_primask );
// irqs reenabled
    }
}

void U_Print(char* s)
{
	str_buffer_enqueue( &str_buf, s );
    U_PrintNow();
}
void U_PrintLn(char* s)
{
	uint8_t len = strlen( (const char*)s );
	char my_str[len+3]; // space for escape sequence
	
	strcpy( my_str, s );
	strcpy( &my_str[len], "\n\r\0" );
    
    str_buffer_enqueue( &str_buf, my_str );
    U_PrintNow();
}
void U_PrintU32(uint32_t n)
{
	static char str[13] = "0xFFFFFFFF\n\r\0";
	for( int8_t i=7; i >= 0; i-- ){
	    uint32_t temp;
		temp = n >> (i<<2);
		temp &= 0x0000000F; // mask lowest nibble
		if( temp < 10 ) { // numeric
			str[9-i] = 48 + (char)temp;
		} else { // alpha
			str[9-i] = 55 + (char)temp;
		}
	}
    U_Print( str );
}
void U_PrintU16(uint16_t n)
{
	static char str[9] = "0xFFFF\n\r\0";
	for( int8_t i=3; i >= 0; i-- ){
	    uint32_t temp;
		temp = n >> (i<<2);
		temp &= 0x000F; // mask lowest nibble
		if( temp < 10 ) { // numeric
			str[5-i] = 48 + (char)temp;
		} else { // alpha
			str[5-i] = 55 + (char)temp;
		}
	}
    U_Print( str );
}
void U_PrintU8(uint8_t n)
{
	static char str[5] = "FF\n\r\0";
	for( int8_t i=1; i >= 0; i-- ){
	    uint32_t temp;
		temp = n >> (i<<2);
		temp &= 0x0F; // mask lowest nibble
		if( temp < 10 ) { // numeric
			str[1-i] = 48 + (char)temp;
		} else { // alpha
			str[1-i] = 55 + (char)temp;
		}
	}
    U_Print( str );
}

// New school func calls
void U_PrintVar( char*    name
               , uint32_t n
			   , uint8_t  ret_flag
			   )
{
	char str[24];
	uint8_t len = strlen( (const char*)name );
	if( len > 10 ){ len = 10; }
	uint8_t i;
	for( i=0; i < len; i++ ){
		str[i] = name[i];
	}
	// add separator
	strcpy( &str[i], ": 0x" );
	i += 4;
	// print hex readout
	for( int8_t j=7; j >= 0; j-- ){
	    uint32_t temp;
		temp = n >> (j<<2);
		temp &= 0x0000000F; // mask lowest nibble
		if( temp < 10 ) { // numeric
			str[(i+7)-j] = 48 + (char)temp;
		} else { // alpha
			str[(i+7)-j] = 55 + (char)temp;
		}
	}
	if( ret_flag ){
		strcpy( &str[i+8], "\n\r\0" ); // carriage return
	} else {
		strcpy( &str[i+8], ", \0" ); // add space
	}
    U_Print( str );
}
#endif // DEBUG
