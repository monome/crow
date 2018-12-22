#include "debug_usart.h"

#include <string.h>

#include "str_buffer.h"

USART_HandleTypeDef handusart;
str_buffer_t str_buf;

#ifdef RELEASE
void Debug_USART_Init(void){ return; }
void U_PrintNow(void){ return; }
void U_Print(char* s){ return; }
void U_PrintLn(char* s){ return; }
void U_PrintU32(uint32_t n){ return; }
void U_PrintU32n(uint32_t n){ return; }
void U_PrintU16(uint16_t n){ return; }
void U_PrintU16n(uint16_t n){ return; }
void U_PrintU8(uint8_t n){ return; }
void U_PrintU8n(uint8_t n){ return; }
void U_PrintF(float n){ return; }
void U_PrintFn(float n){ return; }
void U_PrintVar(char* name, uint32_t n, uint8_t ret_flag){ return; }
#endif // RELEASE

#ifndef RELEASE
void Debug_USART_Init( void )
{
	handusart.Instance = DBG_USARTx;

	handusart.Init.BaudRate   = DBG_USART_baud;
	handusart.Init.WordLength = USART_WORDLENGTH_8B;
	handusart.Init.StopBits   = USART_STOPBITS_1;
	handusart.Init.Parity     = USART_PARITY_NONE;
	handusart.Init.Mode       = USART_MODE_TX;

	HAL_USART_Init( &handusart );

	str_buffer_init( &str_buf, 511 ); // fifo for DMA buffer
}

void Debug_USART_DeInit(void)
{
	HAL_USART_DeInit( &handusart );
	str_buffer_deinit( &str_buf );
}

// LOW LEVEL USART HARDWARE CONFIGURATION FUNCTION
void HAL_USART_MspInit(USART_HandleTypeDef *hu )
{
    if(hu == &handusart){
	    static DMA_HandleTypeDef hdma_tx;

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

        // Configure NVIC for DMA
        HAL_NVIC_SetPriority( USARTx_DMA_TX_IRQn
                            , USARTx_DMA_IRQPriority
                            , USARTx_DMA_TXIRQSubPriority
                            );
	    HAL_NVIC_EnableIRQ( USARTx_DMA_TX_IRQn );

        HAL_NVIC_SetPriority( USARTx_IRQn
                            , USARTx_IRQSubPriority
                            , USARTx_IRQSubPriority
                            );
	    HAL_NVIC_EnableIRQ( USARTx_IRQn );
    }
}

void USARTx_DMA_TX_IRQHandler( void )
{
	HAL_DMA_IRQHandler( handusart.hdmatx );
}

void HAL_USART_TxCpltCallback( USART_HandleTypeDef *husart )
{
}

void HAL_USART_ErrorCallback( USART_HandleTypeDef *husart )
{
    U_Print("errorcode: ");
    U_PrintU8( husart->ErrorCode );
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
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
	str_buffer_enqueue( &str_buf, s );
__set_PRIMASK( old_primask );
}
void U_PrintLn(char* s)
{
	uint8_t len = strlen( (const char*)s );
	char my_str[len+3]; // space for escape sequence
	
	strcpy( my_str, s );
	strcpy( &my_str[len], "\n\r" );
    
    U_Print( my_str );
}
void U_PrintU32(uint32_t n)
{
	char str[] = "0xFFFFFFFF";
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
    U_PrintLn( str );
}
void U_PrintU32n(uint32_t n)
{
	char str[] = "0xFFFFFFFF";
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
	char str[] = "0xFFFF";
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
    U_PrintLn( str );
}
void U_PrintU16n(uint16_t n)
{
	char str[] = "0xFFFF";
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
	char str[] = "FF";
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
    U_PrintLn( str );
}
void U_PrintU8n(uint8_t n)
{
	char str[] = "FF";
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
void U_PrintF(float n)
{
	char str[] = "+000.000";
    if(n >= 999.999){ strcpy( str, "+999.999" ); }
    else if(n <= -999.999){ strcpy( str, "-999.999" ); }
    else{
        if( n >= 0.0 ){
            str[0] = '+';
        } else {
            str[0] = '-';
            n = -n;
        }
        str[1] = 48 + (int)(n / 100);
        str[2] = 48 + (int)(n / 10) % 10;
        str[3] = 48 + (int)(n) % 10;
        str[5] = 48 + (int)(n * 10) % 10;
        str[6] = 48 + (int)(n * 100) % 10;
        str[7] = 48 + (int)(n * 1000) % 10;
    }
    U_PrintLn( str );
}
void U_PrintFn(float n)
{
	char str[] = "+000.000";
    if(n >= 999.999){ strcpy( str, "+999.999" ); }
    else if(n <= -999.999){ strcpy( str, "-999.999" ); }
    else{
        if( n >= 0.0 ){
            str[0] = '+';
        } else {
            str[0] = '-';
            n = -n;
        }
        str[1] = 48 + (int)(n / 100);
        str[2] = 48 + (int)(n / 10) % 10;
        str[3] = 48 + (int)(n) % 10;
        str[5] = 48 + (int)(n * 10) % 10;
        str[6] = 48 + (int)(n * 100) % 10;
        str[7] = 48 + (int)(n * 1000) % 10;
    }
    U_Print( str );
}
#endif // DEBUG
