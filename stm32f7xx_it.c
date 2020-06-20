#include "stm32f7xx_it.h"

#include "stm32f7xx_hal.h" // HAL_IncTick

volatile int CPU_count = 0;

int counts[8] = {0,0,0,0,0,0,0,0};
int pCount = 0;
void SysTick_Handler(void)
{
    HAL_IncTick();
    counts[pCount] = CPU_count;
    if( (++pCount) >= 8 ){ pCount = 0; }
    CPU_count = 0;
}

int CPU_GetCount( void )
{
    int c = 0;
    for( int i=0; i<8; i++ ){
        c += counts[i];
    }
    return c;
}

// In RELEASE mode, all Fault Handlers are ignored and we hope for the best

#ifndef RELEASE // DEBUG mode

#include <stdio.h> // printf
#include "ll/debug_usart.h" // U_PrintNow

static void error( char* msg ){
    printf("%s\n", msg);
    U_PrintNow();
    while(1);
}
void NMI_Handler(void){ error("!NMI"); }
void MemManage_Handler(void){ HardFault_Handler(); error("!MemManage"); }
void BusFault_Handler(void){ error("!BusFault"); }
void UsageFault_Handler(void){ error("!UsageFault"); }
void SVC_Handler(void){ error("!SVC"); }
void DebugMon_Handler(void){ error("!DebugMon"); }
void PendSV_Handler(void){ error("!PendSV"); }


// NB: the below breaks with -flto CFLAG
// The fault handler implementation calls a function called
//   prvGetRegistersFromStack().
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
// These are volatile to try and prevent the compiler/linker optimising them
// away as the variables never actually get used.  If the debugger won't show the
// values of the variables, make them global my moving their declaration outside
// of this function.
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr; // Link register.
    volatile uint32_t pc; // Program counter.
    volatile uint32_t psr;// Program status register.

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    printf("r0  %x\n",(int)r0);
    printf("r1  %x\n",(int)r1);
    printf("r2  %x\n",(int)r2);
    printf("r3  %x\n",(int)r3);
    printf("r12 %x\n",(int)r12);
    printf("lr  %x\n",(int)lr);
    printf("pc  %x\n",(int)pc);
    printf("psr %x\n",(int)psr);
    /* When the following line is hit, the variables contain the register values. */
    error("HardFault");
}
#endif // DEBUG
