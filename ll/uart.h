#pragma once

#include "stm32f7xx_hal.h"

typedef enum{
    UART_Rx,
    UART_Tx,
} UART_Direction;

typedef void (*U8_Callback)(uint8_t);

typedef struct{
    UART_HandleTypeDef hUart;
    U8_Callback cb_handler;
} Uart;

Uart* UART_init(Uart* self, UART_Direction dir, USART_TypeDef* instance, char* pin);

void UART_set_callback(Uart* self, U8_Callback callback);

void UART_send(Uart* self, uint8_t* data, size_t len);
