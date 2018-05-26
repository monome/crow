#include "main.h"

//#include <lua.h>
//#include <lauxlib.h>
//#include <lualib.h>

#include <string.h>

#include "ll/debug_usart.h"
#include "ll/dac8565.h"
#include "ll/ads131.h"

static GPIO_InitTypeDef GPIO_InitStruct;

static void Lua_Test(void);
static void Dbg_Pin_Init(void);
static void Sys_Clk_Config(void);
static void Error_Handler(void);

int main(void)
{
    HAL_Init();
    Sys_Clk_Config();

    Dbg_Pin_Init();
    Debug_USART_Init();
    //Debug_USART_printf("\ntest\n\r");
    //U_PrintLn("crow");
    DAC_Init();
    ADC_Init();

    Lua_Test();

    U_PrintNow();

    DAC_Set( DAC_ALL_CHANNELS, DAC_ZERO_VOLTS );
    ADC_Get(0);
    //ADC_Get(0);
    U_PrintNow();

    uint16_t dack = 0;
    while (1){
        U_PrintU16(ADC_Get(0));
        DAC_Set(0, dack);
        dack++;
    }
}

// Lua deactivated to decrease flash time
static void Lua_Test(void)
{
    /*
    lua_State* L;         // pointer to store lua state
    L = luaL_newstate();  // create a new lua state & save pointer
    luaL_openlibs(L);     // give our lua state access to lua libs

    // lua hello-world as a string
    char* slua =
        "function f()\n                  \
            print(\"Hello from lua\")\n  \
        end";
    luaL_loadbuffer(L, slua
                     , strlen(slua)
                     , "sscript"
                     );
    int result = lua_pcall(L, 0, 0, 0);
    if( result ){ fprintf(stderr, "failure\n"); }
    */
}

static void Dbg_Pin_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
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
    RCC_OscInitStruct.PLL.PLLN       = 336;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ       = 7;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_HCLK
                                | RCC_CLOCKTYPE_PCLK1
                                | RCC_CLOCKTYPE_PCLK2
                                ;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK){
        Error_Handler();
    }

    HAL_RCC_MCOConfig( RCC_MCO2
                     , RCC_MCO2SOURCE_SYSCLK
                     , RCC_MCODIV_2
                     );
}

static void Error_Handler(void)
{
    // print debug message after USART is running
    while(1){;;}
}
