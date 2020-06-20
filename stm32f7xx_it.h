#pragma once

#include <stdint.h>

extern volatile int CPU_count;
int CPU_GetCount( void );
void SysTick_Handler(void);

#ifndef RELEASE // DEBUG
void HardFault_Handler( void ) __attribute__( ( naked ) );

void NMI_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
#endif
