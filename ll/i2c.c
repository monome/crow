#include "i2c.h"

#include <stm32f7xx_hal.h>
#include <string.h>

#include "lib/caw.h" // Caw_send_luachunk (pullup suggestion)

////////////////////////////
// type definitions

// can probably replace with HAL state machine?
typedef enum{ OP_NULL
            , OP_ERROR
            , OP_LISTEN
            , OP_LEAD_TX
            , OP_LEAD_RX
            , OP_FOLLOW_TX
            , OP_FOLLOW_TX_READY
            , OP_FOLLOW_RX
            , OP_ABORT
} I2C_OP_t;

typedef struct{
    I2C_OP_t operation;
    uint8_t  address;
    uint8_t  size;
    uint8_t  command; // store the request
    uint8_t  data[I2C_MAX_CMD_BYTES];
} I2C_State_t;


///////////////////////////////////////
// global vars & objects

I2C_HandleTypeDef i2c_handle;

I2C_lead_callback_t   lead_response;
I2C_follow_callback_t follow_action;
I2C_follow_callback_t follow_request;
I2C_error_callback_t  error_action;

I2C_State_t buf;
uint8_t pullup_state = 0;


//////////////////////////////
// public definitions

uint8_t I2C_Init( uint8_t               address
                , I2C_lead_callback_t   lead_callback
                , I2C_follow_callback_t follow_action_callback
                , I2C_follow_callback_t follow_request_callback
                , I2C_error_callback_t  error_callback
                )
{
    uint8_t error = 0;

    lead_response  = lead_callback;
    follow_action  = follow_action_callback;
    follow_request = follow_request_callback;
    error_action   = error_callback;

    i2c_handle.Instance              = I2Cx;
    i2c_handle.Init.Timing           = I2C_TIMING;
    i2c_handle.Init.OwnAddress1      = address << 1; // correct MSB justification
    i2c_handle.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    i2c_handle.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    i2c_handle.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    i2c_handle.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

    if( HAL_I2C_Init( &i2c_handle ) != HAL_OK ){ error = 1; }

    buf.operation = OP_NULL;
    buf.address   = 0;
    buf.size      = 0;
    buf.command   = 0;
    memset( buf.data, 0, I2C_MAX_CMD_BYTES );

    HAL_I2C_ListenCpltCallback( &i2c_handle );
    return error;
}

void I2C_DeInit( void )
{
    HAL_I2C_DeInit( &i2c_handle );
}

uint8_t I2C_is_boot( void )
{
    uint8_t boot = 0;
    GPIO_InitTypeDef gpio;
    I2Cx_SCL_GPIO_CLK_ENABLE();
    gpio.Pin   = I2Cx_SCL_PIN;
    gpio.Mode  = GPIO_MODE_INPUT;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( I2Cx_SCL_GPIO_PORT, &gpio );

    if( !HAL_GPIO_ReadPin( I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN ) ){
        boot = 1;
    }

    // set to OD to ensure no interaction with i2c line
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( I2Cx_SCL_GPIO_PORT, &gpio );

    HAL_GPIO_DeInit( I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN );
    return boot;
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

void I2C_SetPullups( uint8_t state )
{
    pullup_state = state;

    GPIO_InitTypeDef gpio;
    gpio.Pin       = I2Cx_SCL_PIN
                   | I2Cx_SDA_PIN;
    gpio.Mode      = GPIO_MODE_AF_OD;
    gpio.Pull      = (state) ? GPIO_PULLUP : GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = I2Cx_SCL_SDA_AF;

    HAL_GPIO_DeInit( I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN );
    HAL_GPIO_DeInit( I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN );
    HAL_GPIO_Init( I2Cx_SCL_GPIO_PORT, &gpio );
}

uint8_t I2C_GetPullups( void )
{
    return pullup_state;
}

uint8_t I2C_GetAddress( void )
{
    return i2c_handle.Init.OwnAddress1 >> 1;
}

void I2C_SetAddress( uint8_t address )
{
    i2c_handle.Init.OwnAddress1 = address << 1;
}


////////////////////////////
// leader set/get

#define I2C_READY_TIMEOUT 20000
int timeout = I2C_READY_TIMEOUT;

int I2C_is_ready( void )
{
    switch( buf.operation ){
        case OP_LISTEN:
            timeout = I2C_READY_TIMEOUT;
            break;

        case OP_ABORT:
        case OP_NULL:
            // waiting
            break;

        default: // all others
            if( --timeout <= 0 ){
                buf.operation = OP_ABORT;
                HAL_I2C_Master_Abort_IT( &i2c_handle, buf.address );
            }
            break;
    }
    return (buf.operation == OP_LISTEN);
}

int I2C_LeadTx( uint8_t  address
              , uint8_t* data
              , uint8_t  size
              )
{
    address <<= 1;
    int error = 0;
    if( buf.operation == OP_LISTEN ){
        buf.address = address; // save in case of abort
        BLOCK_IRQS(
            if( HAL_I2C_DisableListen_IT( &i2c_handle ) != HAL_OK ){ error |= 1; }
            if( HAL_I2C_Master_Transmit_IT( &i2c_handle
                    , address
                    , data
                    , size
                    ) != HAL_OK ){
                error |= 2;
                HAL_I2C_ListenCpltCallback( &i2c_handle ); // re-enable listen
            } else { buf.operation = OP_LEAD_TX; }
        );
    } else {
        printf("can't leadTx because operation=%i\n",buf.operation);
        error = 1;
    }
    return error;
}

int I2C_LeadRx( uint8_t  address
              , uint8_t* data
              , uint8_t  size
              , uint8_t  rx_size
              )
{
    address <<= 1;
    int error = 0;
    if( buf.operation == OP_LISTEN ){
        buf.address = address;
        buf.size    = rx_size;
        buf.command = data[0];
        BLOCK_IRQS(
            if( HAL_I2C_DisableListen_IT( &i2c_handle ) != HAL_OK ){
                error |= 0x1;
            } else if( __HAL_I2C_GET_FLAG( &i2c_handle, I2C_FLAG_BUSY ) ){
                // NB: Explicitly check BUSY flag as Sequential_Transmit doesn't
                error |= 0x2;
                HAL_I2C_ListenCpltCallback( &i2c_handle ); // re-enable listen
            } else if( HAL_I2C_Master_Sequential_Transmit_IT( &i2c_handle
                            , address
                            , data
                            , size
                            , I2C_FIRST_FRAME
                            ) != HAL_OK ){
                error |= 0x4;
                HAL_I2C_ListenCpltCallback( &i2c_handle ); // re-enable listen
            } else { buf.operation = OP_LEAD_RX; }
        );
    } else {
        printf("can't leadRx because operation=%i\n",buf.operation);
        error = 8;
    }
    return error;
}



////////////////////////////////////////////////
// HAL callbacks
#define I2C_NO_OPTION_FRAME     (0xFFFF0000U)
void I2Cx_EV_IRQHandler( void )
{
    // Overload the HAL EV callback to handle early STOPF flag for variable length
    // i2c messagess. And (TODO) handle the arbitration-lost flag directly.

    if( __HAL_I2C_GET_FLAG( &i2c_handle, I2C_FLAG_STOPF ) ){
        if( buf.operation == OP_FOLLOW_RX ){ // End of variable length reception
        // HACK to make the EV Handler treat STOPF as a non-error rx
        // this whole runaround is probably bc shouldn't use sequential rxtx
            i2c_handle.XferCount = 0; // pretend we're done
            i2c_handle.XferOptions = I2C_NO_OPTION_FRAME;
            i2c_handle.State = HAL_I2C_STATE_BUSY_RX;
        } else if( buf.operation == OP_FOLLOW_TX ){ printf("tx STOPF\n"); }
    }
    if( __HAL_I2C_GET_FLAG( &i2c_handle, I2C_FLAG_ARLO ) ){
        // happens when multiple modules returning different data to TT
        // the losing module seems to cancel it's transmission
        // just need to check that rxing & txing are in a good state
        // also make sure any lib functions are cancelled
        printf("TODO Arbitration Lost\n");
    }
    HAL_I2C_EV_IRQHandler( &i2c_handle );
    // TODO EV handler will unset an Ack Failure flag but this seems to always
    // happen in some non-error cases. perhaps it makes sense to check here, after
    // we've run the callback and see if the flags remain?
    // perhaps all the flags are crushed by the event handler?
    if( __HAL_I2C_GET_FLAG( &i2c_handle, I2C_FLAG_AF ) ){
        //printf("TODO Ack failure\n");
    }
}


/////////////////////////////////////
// lead

// finished sending
void HAL_I2C_MasterTxCpltCallback( I2C_HandleTypeDef* h )
{
    if( buf.operation == OP_LEAD_RX ){
        BLOCK_IRQS(
            //if( HAL_I2C_Master_Receive_IT( &i2c_handle
            if( HAL_I2C_Master_Sequential_Receive_IT( &i2c_handle
                    , buf.address
                    , buf.data
                    , buf.size
                    , I2C_LAST_FRAME
                    ) != HAL_OK ){ printf("LeadRx failed\n"); }
        );
    } else { // buf.operation == OP_LEAD_TX
        HAL_I2C_ListenCpltCallback( &i2c_handle );
    }
}

// received response from follower
void HAL_I2C_MasterRxCpltCallback( I2C_HandleTypeDef* h )
{
    (*lead_response)( buf.address>>1
                    , buf.command
                    , buf.data
                    );
    HAL_I2C_ListenCpltCallback(h);
}


/////////////////////////////////////
// follow

// follower action/request received
void HAL_I2C_AddrCallback( I2C_HandleTypeDef* h
                         , uint8_t            TransferDirection
                         , uint16_t           AddrMatchCode
                         )
{
    uint8_t error = 0;
    if( TransferDirection ){ // Request data
        buf.size = (*follow_request)( buf.data );
        if( !buf.size ){
            printf("TODO: return_size==0 -> abort sequential tx\n");
        }
        buf.operation = OP_FOLLOW_TX_READY;
        error = HAL_I2C_Slave_Sequential_Transmit_IT( &i2c_handle
                , buf.data
                , buf.size
                , I2C_LAST_FRAME
                );
        if( error ){ printf("ftx failed: %i\n", error); }
        else{ buf.operation = OP_FOLLOW_TX; }
    } else {
        BLOCK_IRQS(
            error = HAL_I2C_Slave_Sequential_Receive_IT( &i2c_handle
                    , buf.data
                    , I2C_MAX_CMD_BYTES
                    , I2C_NEXT_FRAME
                    );
        );
        if( error ){ printf("frx failed: %i\n", error); }
        else { buf.operation = OP_FOLLOW_RX; }
    }
}

void HAL_I2C_SlaveRxCpltCallback( I2C_HandleTypeDef* h )
{
    // triggered by STOPF flag in EV_IrqHandler
    (*follow_action)( buf.data );
    HAL_I2C_ListenCpltCallback( &i2c_handle );
}

void HAL_I2C_SlaveTxCpltCallback( I2C_HandleTypeDef* h )
{
    buf.operation = OP_LISTEN;
}

// follower action/request completed
void HAL_I2C_ListenCpltCallback( I2C_HandleTypeDef* hi2c )
{
    BLOCK_IRQS(
        if( HAL_I2C_EnableListen_IT( &i2c_handle ) != HAL_OK ){
            printf("i2c enable listen failed: %x\n", hi2c->State);
        } else { buf.operation = OP_LISTEN; }
    );
}


////////////////////////////////////////////
// error handling

void I2Cx_ER_IRQHandler( void )
{
    buf.operation = OP_ERROR;
    HAL_I2C_ER_IRQHandler( &i2c_handle );
}

void HAL_I2C_ErrorCallback( I2C_HandleTypeDef* h )
{
    if( h->ErrorCode == HAL_I2C_ERROR_AF ){
        (*error_action)( 0 );
    } else {
        if( h->ErrorCode == 2 ){
            (*error_action)( 1 );
        } else{
            (*error_action)( (int)h->ErrorCode );
        }
    }
    HAL_I2C_ListenCpltCallback( &i2c_handle ); // Re-enable Listen
}

void HAL_I2C_AbortCpltCallback( I2C_HandleTypeDef* h )
{
    timeout = I2C_READY_TIMEOUT;
    HAL_I2C_ListenCpltCallback( &i2c_handle );
}
