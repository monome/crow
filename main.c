#include "main.h"

#include <string.h>

#include "lib/bootloader.h" // bootloader_is_i2c_force()
#include "lib/io.h"
#include "lib/caw.h"
#include "lib/ii.h"
#include "lib/lualink.h"

#include "ll/debug_usart.h"
#include "ll/debug_pin.h"
#include "ll/midi.h"


static void Sys_Clk_Config(void);
static void Error_Handler(void);

int main(void)
{
    HAL_Init();
    Sys_Clk_Config();

    bootloader_is_i2c_force();

    Debug_Pin_Init();
    Debug_USART_Init();
    U_PrintLn("\n\rcrow"); U_PrintNow();

    IO_Init();

    Caw_Init();
    U_PrintLn("caw");

    Lua_Init(); // send this function a list of fnptrs?

    //MIDI_Init();
    //II_init( II_FOLLOW );

    Lua_crowbegin();
    IO_Start(); // buffers need to be ready by now

    while(1){
        U_PrintNow();
        switch( Caw_try_receive() ){ // true on pressing 'enter'
            case 1:
                Lua_repl( Caw_get_read()
                        , Caw_get_read_len() // len is ignored for \0 anyway
                        , Caw_send_luaerror // 'print' continuation
                        );
                Caw_send_raw( (uint8_t*)"\n> \0", 4 );
                break;
            case 2: bootloader_enter(); break;
            default: break;
        }
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

static void Error_Handler(void)
{
    // print debug message after USART is running
    U_PrintLn("Error Handler");
    U_PrintNow();
    while(1){;;}
}
