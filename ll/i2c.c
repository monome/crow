#include "i2c.h"

#include <stm32f7xx_hal.h>

#include "debug_usart.h"

I2C_HandleTypeDef i2c_handle;

typedef struct{
	uint8_t  b_count;  // # of buffer slots active
	uint8_t  b_head;   // index of 1st active buffer slot
    volatile uint8_t  rxing;    // currently waiting for rx callback
    uint8_t* tx_data;  // ptr to data to transmit
    uint8_t  tx_bytes; // length of tx data in bytes

    volatile uint8_t  txing;
    uint8_t           lead_rx_address;
    uint8_t           lead_rx_cmd;
    uint8_t*          lead_rx_data;
    uint8_t           lead_rx_bytes;

	uint8_t buffer[I2C_BUFFER_LEN][I2C_MAX_CMD_BYTES];
} i2c_state_t;
i2c_state_t        i2c_state;

// private declarations
void _I2C_FollowRx( I2C_HandleTypeDef* h );
uint8_t* _I2C_GetBuffer( i2c_state_t* state );

// public definitions
uint8_t I2C_Init( uint8_t address )
{
    uint8_t error = 0;

	i2c_handle.Instance              = I2Cx;
	i2c_handle.Init.Timing           = I2C_TIMING;
	i2c_handle.Init.OwnAddress1      = address;
	i2c_handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
	i2c_handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
	i2c_handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
	i2c_handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

	if( HAL_I2C_Init( &i2c_handle ) != HAL_OK ){ error = 1; }

	i2c_state.b_count  = 0;
	i2c_state.b_head   = 0;
    i2c_state.rxing    = 0;
    i2c_state.tx_data  = NULL;
    i2c_state.tx_bytes = 0;
	for( uint8_t i=0; i<I2C_BUFFER_LEN; i++ ){
		for( uint8_t j=0; j<I2C_MAX_CMD_BYTES; j++ ){
			i2c_state.buffer[i][j] = 0;
		}
	}
    i2c_state.txing           = 0;
    i2c_state.lead_rx_address = 0;
    i2c_state.lead_rx_data    = NULL;
    i2c_state.lead_rx_bytes   = 0;

uint32_t old_primask = __get_PRIMASK();
__disable_irq();
	if( HAL_I2C_EnableListen_IT( &i2c_handle ) != HAL_OK ){ error = 2; }
__set_PRIMASK( old_primask );
    
    return error;
}

uint8_t I2C_is_boot( void )
{
    uint8_t boot = 0;
	GPIO_InitTypeDef gpio;
	I2Cx_SCL_GPIO_CLK_ENABLE();
	gpio.Pin       = I2Cx_SCL_PIN;
	gpio.Mode      = GPIO_MODE_INPUT;
	gpio.Pull      = GPIO_PULLUP;
	gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( I2Cx_SCL_GPIO_PORT, &gpio );

    if( !HAL_GPIO_ReadPin( I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN ) ){
        boot = 1;
    }

    // set to OD to ensure no damage by i2c line
    // FIXME is this necessary, or does DeInit set the pin to tristate?
	gpio.Mode      = GPIO_MODE_AF_OD;
	HAL_GPIO_Init( I2Cx_SCL_GPIO_PORT, &gpio );

	HAL_GPIO_DeInit( I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN );
    return boot;
}

void I2C_DeInit( void )
{
    HAL_I2C_DeInit( &i2c_handle );
}

void HAL_I2C_MspInit( I2C_HandleTypeDef* h )
{
	GPIO_InitTypeDef gpio;
	I2Cx_SCL_GPIO_CLK_ENABLE();
	I2Cx_SDA_GPIO_CLK_ENABLE();
	I2Cx_CLK_ENABLE();
	
	gpio.Pin       = I2Cx_SCL_PIN
	               | I2Cx_SDA_PIN;
	gpio.Mode      = GPIO_MODE_AF_OD;
	gpio.Pull      = GPIO_NOPULL;
	gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
	gpio.Alternate = I2Cx_SCL_SDA_AF;
	HAL_GPIO_Init( I2Cx_SCL_GPIO_PORT, &gpio );

	// interrupt mode
	HAL_NVIC_SetPriority( I2Cx_ER_IRQn
                        , I2Cx_ER_Priority
                        , I2Cx_ER_SubPriority
                        );
	HAL_NVIC_EnableIRQ( I2Cx_ER_IRQn );

	HAL_NVIC_SetPriority( I2Cx_EV_IRQn
                        , I2Cx_EV_Priority
                        , I2Cx_EV_SubPriority
                        );
	HAL_NVIC_EnableIRQ( I2Cx_EV_IRQn );
}

void HAL_I2C_MspDeInit( I2C_HandleTypeDef* h )
{
	I2Cx_FORCE_RESET();
	I2Cx_RELEASE_RESET();

	HAL_NVIC_DisableIRQ( I2Cx_ER_IRQn );
	HAL_NVIC_DisableIRQ( I2Cx_EV_IRQn );

	HAL_GPIO_DeInit( I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN );
	HAL_GPIO_DeInit( I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN );
}

void HAL_I2C_ErrorCallback( I2C_HandleTypeDef* h )
{
    if( h->ErrorCode == HAL_I2C_ERROR_AF ){
	    U_PrintLn("I2C_ERROR_AF");
        // Seems to lock up i2c bus until whole driver is reset
        // Software reset
/* A software reset can be performed by clearing the PE bit in the I2C_CR1 register. In that
case I2C lines SCL and SDA are released. Internal states machines are reset and
communication control bits, as well as status bits come back to their reset value. The
configuration registers are not impacted.
Here is the list of impacted register bits:
1.
 I2C_CR2 register: START, STOP, NACK
2.
 I2C_ISR register: BUSY, TXE, TXIS, RXNE, ADDR, NACKF, TCR, TC, STOPF, BERR,
ARLO, OVR
and in addition when the SMBus feature is supported:
1.
 I2C_CR2 register: PECBYTE
2.
 I2C_ISR register: PECERR, TIMEOUT, ALERT
PE must be kept low during at least 3 APB clock cycles in order to perform the software
reset. This is ensured by writing the following software sequence: - Write PE=0 - Check
PE=0 - Write PE=1.
*/
        uint8_t temp_addr = h->Init.OwnAddress1;
	    I2C_DeInit();
        I2C_Init( temp_addr );
    } else {
	    //U_PrintLn("I2C_ERROR");
	    //U_PrintU32( (uint32_t)h->ErrorCode );
    }
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
	if( HAL_I2C_EnableListen_IT( &i2c_handle ) != HAL_OK ){
        U_PrintLn("enable listen failed");
    }
__set_PRIMASK( old_primask );
}
void HAL_I2C_AddrCallback( I2C_HandleTypeDef* h
	                     , uint8_t            TransferDirection
	                     , uint16_t           AddrMatchCode
	                     )
{
    if( TransferDirection ){ // Request data
        // change direction in follow mode to respond to request
        while( i2c_state.rxing ){;;} // wait for cmd to be received
            // add a timeout     ^^
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
        if( HAL_I2C_Slave_Sequential_Transmit_IT( &i2c_handle
                , i2c_state.tx_data
                , i2c_state.tx_bytes
                , I2C_FIRST_AND_LAST_FRAME
                ) != HAL_OK ){
            //U_PrintLn("i2c seq tx failed");
        }
__set_PRIMASK( old_primask );
    } else {
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
        if( HAL_I2C_Slave_Sequential_Receive_IT( &i2c_handle
                , _I2C_GetBuffer( &i2c_state )
                , I2C_MAX_CMD_BYTES
                , I2C_NEXT_FRAME
	            ) != HAL_OK ){
	        //U_PrintLn("i2c seq rx failed");
	    }
__set_PRIMASK( old_primask );
        i2c_state.rxing = 1;
    }
}

void HAL_I2C_ListenCpltCallback( I2C_HandleTypeDef* hi2c )
{
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
	if( HAL_I2C_EnableListen_IT( &i2c_handle ) != HAL_OK ){
	    //U_PrintLn("i2c enable listen failed");
	}
__set_PRIMASK( old_primask );
}
void HAL_I2C_MasterTxCpltCallback( I2C_HandleTypeDef* h )
{
    if( i2c_state.txing == 1 ){
        //U_PrintLn("lead_tx -> rx");
        // leader has transmitted a request
        // now ready to leader_receive the data
        i2c_state.txing = 2;
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
        if( HAL_I2C_Master_Sequential_Receive_IT( &i2c_handle
                , i2c_state.lead_rx_address
		        , i2c_state.lead_rx_data
	            , i2c_state.lead_rx_bytes
                , I2C_NEXT_FRAME
	            ) != HAL_OK ){ U_PrintLn("LeadRx"); }
__set_PRIMASK( old_primask );
    } else {
        //U_PrintLn("lead_tx");
        // leader_transmission has completed
        // return to follower_listen state
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
        if( HAL_I2C_EnableListen_IT( &i2c_handle ) != HAL_OK ){
		    U_PrintLn("i2c enable listen failed");
	    }
__set_PRIMASK( old_primask );
    }
}
void HAL_I2C_MasterRxCpltCallback( I2C_HandleTypeDef* h )
{
    //U_PrintLn("lead_rx");
    // last stage of a master receive

    i2c_state.txing = 0;
    I2C_RxCpltCallback( i2c_state.lead_rx_address>>1
                      , i2c_state.lead_rx_cmd
                      , i2c_state.lead_rx_data
                      );
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
    if( HAL_I2C_EnableListen_IT( &i2c_handle ) != HAL_OK ){
        U_PrintLn("i2c enable listen failed");
	}
__set_PRIMASK( old_primask );
}

void HAL_I2C_SlaveRxCpltCallback( I2C_HandleTypeDef* h )
{
    // does *not* occur when command sent from TT
    // prob doesn't ever occur as we always allow endless transmission
    // instead callback is triggered by a STOPF flag in EV_IrqHandler
    //U_Print("follow_rx"); // left on to see if it ever occurs
    I2C_RxCpltCallback( 0x0, 0x0, _I2C_GetBuffer( &i2c_state ) );
    i2c_state.rxing = 0; // expecting data now
}
void HAL_I2C_SlaveTxCpltCallback( I2C_HandleTypeDef* h )
{
    // called when TT requests value
    //U_Print("slave_tx");
}

void I2Cx_EV_IRQHandler( void )
{
    if( __HAL_I2C_GET_FLAG( &i2c_handle, I2C_FLAG_STOPF ) ){
        if( i2c_state.rxing ){
            // end of variable length reception
            // triggered when receiving remote command
            __HAL_I2C_CLEAR_FLAG( &i2c_handle, I2C_FLAG_STOPF );
            HAL_I2C_SlaveRxCpltCallback( &i2c_handle );
        }
    }
    if( __HAL_I2C_GET_FLAG( &i2c_handle, I2C_FLAG_ARLO ) ){
        // happens when multiple w/ modules returning different data to TT
        // the losing module seems to cancel it's transmission
        // just need to check that rxing & txing are in a good state
        // also make sure any lib functions are cancelled
        U_PrintLn("Arbitration Lost");
    }
    if( __HAL_I2C_GET_FLAG( &i2c_handle, I2C_FLAG_AF ) ){
        U_PrintLn("Ack failure");
    }
	HAL_I2C_EV_IRQHandler( &i2c_handle );
}

void I2Cx_ER_IRQHandler( void )
{
    U_Print("ER:");
    U_PrintU8(HAL_I2C_GetError( &i2c_handle ));
    i2c_state.txing = 0;
    i2c_state.rxing = 0;
	HAL_I2C_ER_IRQHandler( &i2c_handle );
}

// public fns
void I2C_SetTxData( uint8_t* data, uint8_t size )
{
    i2c_state.tx_data = data;
    i2c_state.tx_bytes = size;
}

void I2C_BufferRx( uint8_t* data )
{
    i2c_state.b_count++;
}

uint8_t I2C_GetAddress( void )
{
    return i2c_handle.Init.OwnAddress1;
}

void I2C_SetAddress( uint8_t address )
{
	i2c_handle.Init.OwnAddress1 = address;
}

uint8_t* I2C_PopFollowBuffer( void )
{
    uint8_t* retval = i2c_state.buffer[i2c_state.b_head];
	
    i2c_state.b_count--;
	i2c_state.b_head = (i2c_state.b_head + 1) % I2C_BUFFER_LEN;
	
    return retval;
}
uint8_t I2C_FollowBufferNotEmpty( void )
{
    return (i2c_state.b_count != 0);
}
uint8_t* I2C_PopLeadBuffer( void )
{
    static uint8_t* retval;
    retval = i2c_state.lead_rx_data;

    // reset address (for NotEmpty check)
    i2c_state.lead_rx_address = 0;

    return retval;
}
uint8_t I2C_LeadBufferNotEmpty( void )
{
    uint8_t retval = (i2c_state.lead_rx_address != 0);
    retval &= (i2c_state.txing == 0);
    return retval;
}

uint8_t I2C_LeadTx( uint8_t  address
                  , uint8_t* data
                  , uint8_t  size
                  )
{
    address <<= 1;
    //U_PrintLn("leadtx");
    uint8_t error = 0;
    if( HAL_I2C_DisableListen_IT( &i2c_handle ) != HAL_OK ){ error |= 1; }
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
	if( HAL_I2C_Master_Transmit_IT( &i2c_handle
            , address
		    , data
	        , size
	        ) != HAL_OK ){ error |= 2; }
__set_PRIMASK( old_primask );
    return error;
}
/*  this fn should really be broken into parts
 *  1. enqueue the query
 *  2. transmit request info
 *  3. (on callback) receive data
 *  4. (on callback) process data
 */

uint8_t I2C_LeadRx( uint8_t  address
                  , uint8_t* data
                  , uint8_t  size
                  , uint8_t  rx_size
                  )
{
    address <<= 1;
    //U_PrintLn("leadrx");
    uint8_t error = 0;
    i2c_state.txing = 1;
    i2c_state.lead_rx_address = address;
    i2c_state.lead_rx_cmd     = data[0];
    i2c_state.lead_rx_data    = data;//&data[1];
    i2c_state.lead_rx_bytes   = rx_size;

    // before continuing, check if the driver is free

    // below happens in a 'pop' fn (responses are non-blocking on IRQ)
    if( HAL_I2C_DisableListen_IT( &i2c_handle ) != HAL_OK ){ error |= 1; }
uint32_t old_primask = __get_PRIMASK();
__disable_irq();
	if( HAL_I2C_Master_Sequential_Transmit_IT( &i2c_handle
            , address
            , data
	        , size
            , I2C_FIRST_FRAME
	        ) != HAL_OK ){ error |= 2; }
__set_PRIMASK( old_primask );
    return error;
}

// private fns
uint8_t* _I2C_GetBuffer( i2c_state_t* state )
{
    return state->buffer[(i2c_state.b_head+i2c_state.b_count)%I2C_BUFFER_LEN];
}

void _I2C_FollowRx( I2C_HandleTypeDef* h )
{
}
