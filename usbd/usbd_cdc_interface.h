#pragma once

#include "usbd_cdc.h"
#include "../ll/interrupts.h" // USB_IRQPriority

/* Periodically, the state of the buffer "UserTxBuffer" is checked.
   The period depends on CDC_POLLING_INTERVAL */
#define CDC_POLLING_INTERVAL (float)2.0 /* in ms. The max is 65 and the min is 1 */

extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;
extern int timer_index;

void CDC_clear_buffers();

void USB_tx_enqueue( uint8_t* buf, uint32_t len );
size_t USB_tx_space( void );
int USB_tx_is_ready( void );
uint8_t USB_rx_dequeue_LOCK( uint8_t** buf, uint32_t* len );
void USB_rx_dequeue_UNLOCK( void );
