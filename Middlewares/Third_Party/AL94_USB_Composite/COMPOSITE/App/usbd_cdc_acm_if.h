#pragma once

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_acm.h"

extern USBD_CDC_ACM_ItfTypeDef  USBD_CDC_ACM_fops;

#define CDC_POLLING_INTERVAL (float)2.0 /* in ms. The max is 65 and the min is 1 */

uint8_t CDC_Transmit(uint8_t ch, uint8_t* Buf, uint16_t Len);
uint8_t CDC_Transmit_Enqueue(uint8_t ch, uint8_t *Buf, uint16_t Len);
int CDC_Transmit_Is_Ready(void);
size_t CDC_Transmit_Space(void);
int CDC_Receive_Dequeue_LOCK(uint8_t ch, uint8_t** buf, uint32_t* len);
void CDC_Receive_Dequeue_UNLOCK(uint8_t ch);
void CDC_Set_Timer_Index(int timer_index);
void CDC_clear_buffers(void);
