#include <string.h>

#include "lib/bootloader.h" // bootloader_is_i2c_force()
#include "lib/io.h"
#include "lib/caw.h"
#include "lib/events.h"
#include "lib/ii.h"
#include "lib/lualink.h"
#include "lib/repl.h"
#include "lib/metro.h"
#include "lib/flash.h" // Flash_clear_user_script()
#include "syscalls.c" // printf() redirection

#include "ll/debug_usart.h"
#include "ll/debug_pin.h"
#include "ll/midi.h"
#include "ll/random.h"

#include "usbd/usbd_cdc_interface.h"

static void Sys_Clk_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);

int main(void)
{
    MPU_Config();
    CPU_CACHE_Enable();
    HAL_Init();
    Sys_Clk_Config();

    // init debugging
    Debug_Pin_Init();
    Debug_USART_Init(); // ignored in TRACE mode

    printf("\n\nhi from crow!\n");

    // init drivers
    IO_Init();
    events_init();
    Metro_Init();
    Caw_Init();
    MIDI_Init();
    II_init( II_CROW );
    Random_Init();

    REPL_init( Lua_Init() );

    IO_Start(); // must start IO before running lua init() script
    Lua_crowbegin();

    CDC_main_init(); // FIXME: stops crash when starting without usb connected

    event_t e;

    while(1){
        U_PrintNow();
        switch( Caw_try_receive() ){ // true on pressing 'enter'
            case C_repl:       REPL_eval( Caw_get_read()
                                        , Caw_get_read_len()
                                        , Caw_send_luaerror
                                        ); break;
            case C_boot:       bootloader_enter(); break;
            case C_flashstart: REPL_mode( REPL_reception ); break;
            case C_flashend:   REPL_mode( REPL_normal ); break;
            case C_flashclear: Flash_clear_user_script(); break;
            case C_restart:    bootloader_restart(); break;
            case C_print:      REPL_print_script(); break;
            default: break; // 'C_none' does nothing
        }
        Random_Update();
        // check/execute single event
        if (event_next(&e))
            (app_event_handlers)[e.type](&e);
    }
}

static void Sys_Clk_Config(void)
{
    static RCC_ClkInitTypeDef RCC_ClkInitStruct;
    static RCC_OscInitTypeDef RCC_OscInitStruct;

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 8;
    RCC_OscInitStruct.PLL.PLLN       = 432;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 9;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){ Error_Handler(); }

    if(HAL_PWREx_EnableOverDrive() != HAL_OK) { Error_Handler(); }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_HCLK
                                | RCC_CLOCKTYPE_PCLK1
                                | RCC_CLOCKTYPE_PCLK2
                                ;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK){
        Error_Handler();
    }
}

static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	// Disable the MPU
	HAL_MPU_Disable();

	// Configure the MPU attributes as WT for SRAM
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = 0x20020000;
	MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	// Enable the MPU
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void CPU_CACHE_Enable(void)
{
	// Enable I-Cache
	SCB_EnableICache();

	// Enable D-Cache
	SCB_EnableDCache();
}


static void Error_Handler(void)
{
    // print debug message after USART is running
    printf("Error Handler\n");
    U_PrintNow();
    while(1){;;}
}
