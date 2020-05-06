#pragma once

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */

extern volatile int CPU_count;

void HardFault_Handler( void ) __attribute__( ( naked ) );

int CPU_GetCount( void );

void NMI_Handler(void);
//void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
