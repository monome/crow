#include "uart.h"

#include <stdio.h>
#include "../lib/caw.h"
#include "debug_pin.h"

static Uart* selves[2]; // for callbacks
static Uart* _self; // for MSP init

static uint8_t rx_buffer[1] = {0}; // buffer for reception. TODO extend to circular buffer & pass length

Uart* UART_init(Uart* self, UART_Direction dir, USART_TypeDef* instance, char* pin){
    selves[dir] = self; // for interrupt lookup

    self->hUart.Instance = instance;
    self->hUart.Init.BaudRate = 31250;
    self->hUart.Init.WordLength = UART_WORDLENGTH_8B;
    self->hUart.Init.StopBits = UART_STOPBITS_1;
    self->hUart.Init.Parity = UART_PARITY_NONE;
    self->hUart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    self->hUart.Init.OverSampling = UART_OVERSAMPLING_16;

    if(dir == UART_Rx){
        self->hUart.Init.Mode = UART_MODE_RX;
    } else if(dir == UART_Tx){
        self->hUart.Init.Mode = UART_MODE_TX;
    }

    _self = self;
    if(HAL_UART_Init(&self->hUart) != HAL_OK){
        Caw_printf("uart failed to init %i\n\r", dir);
        Debug_Pin_Set(2, 1);
    }
    _self = NULL;

    return self;
}

void UART_set_callback(Uart* self, U8_Callback callback){
    self->cb_handler = callback;

    // start reception loop once callback is registered
    if(HAL_UART_Receive_IT(&self->hUart, rx_buffer, 1) != HAL_OK){
        Caw_printf("failed to start reception\n\r");
        Debug_Pin_Set(2, 1);
    }
}

void UART_send(Uart* self, uint8_t* data, size_t len){
    // FIXME currently blocking!
    // TODO use IRQ mode & ignore response, or use DMA mode if we need longer messages (sysex)
    HAL_UART_Transmit(&self->hUart, data, len, 1000);
// HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
// HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
}


// FIXME all the pin mapping is hard coded right now
// FIXME interrupt priorities. only needed on RX channel
void HAL_UART_MspInit(UART_HandleTypeDef* huart){
    GPIO_InitTypeDef g;
    g.Mode      = GPIO_MODE_AF_PP;
    g.Pull      = GPIO_PULLUP;
    g.Speed     = GPIO_SPEED_FAST;  
    RCC_PeriphCLKInitTypeDef rcc;

    // Select SysClk as source of USART1 clocks
    if(huart->Instance == UART5){
        __GPIOB_CLK_ENABLE(); // FIXME auto-select from _self->pinname
        rcc.PeriphClockSelection = RCC_PERIPHCLK_UART5;
        rcc.Uart5ClockSelection = RCC_UART5CLKSOURCE_SYSCLK;
        HAL_RCCEx_PeriphCLKConfig(&rcc);
        __UART5_CLK_ENABLE();
        g.Pin       = GPIO_PIN_8; // FIXME
        g.Alternate = GPIO_AF7_UART5; // FIXME depends on pin mapping
        HAL_GPIO_Init(GPIOB, &g);
        HAL_NVIC_SetPriority(UART5_IRQn, 3, 1);
        HAL_NVIC_EnableIRQ(UART5_IRQn);
    } else if(huart->Instance == UART8){
        __GPIOE_CLK_ENABLE(); // FIXME auto-select from _self->pinname
        rcc.PeriphClockSelection = RCC_PERIPHCLK_UART8;
        rcc.Uart8ClockSelection = RCC_UART8CLKSOURCE_SYSCLK;
        HAL_RCCEx_PeriphCLKConfig(&rcc);
        __UART8_CLK_ENABLE();
        g.Pin       = GPIO_PIN_1; // FIXME
        g.Alternate = GPIO_AF8_UART8; // FIXME depends on pin mapping
        HAL_GPIO_Init(GPIOE, &g);
        // no callback for TX
        // HAL_NVIC_SetPriority(UART8_IRQn, 3, 0);
        // HAL_NVIC_EnableIRQ(UART8_IRQn);
    }
}

void UART5_IRQHandler(void){
    HAL_UART_IRQHandler(&selves[UART_Rx]->hUart);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* h){
    // UNUSED
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* h){
    selves[UART_Rx]->cb_handler(rx_buffer[0]);
    if(HAL_UART_Receive_IT(&selves[UART_Rx]->hUart, rx_buffer, 1) != HAL_OK){
        Caw_printf("failed to restart reception\n\r");
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* h){
    Caw_printf("UART error %i\n\r", h->ErrorCode);
}
