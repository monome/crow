#include "main.h"

//#include <lua.h>
//#include <lauxlib.h>
//#include <lualib.h>

#include <string.h>

#include "lib/io.h"
#include "lib/caw.h"
#include "lib/ii.h"

#include "ll/debug_usart.h"
#include "ll/debug_pin.h"

#include "usbd/usbd_main.h"

static void Lua_Test(void);
static void Sys_Clk_Config(void);
static void Error_Handler(void);

int main(void)
{
    HAL_Init();
    Sys_Clk_Config();

    Debug_Pin_Init();
    Debug_USART_Init();
    U_PrintLn("\ncrow");

    IO_Init();

    II_init( II_FOLLOW );

    Lua_Test();

    //USB_CDC_Init();

    U_PrintNow();

    IO_Start(); // buffers need to be ready by now

    uint8_t flip = 0;
    float inc = 5.0;
    while(1){
        U_PrintNow();
        //IO_Get(0);
        //Caw_try_receive();
        //HAL_Delay(500); Caw_send_rawtext("caw");
        //HAL_Delay(500); Caw_send_luachunk("listen");
        //inc = flip ? 0.1 : 1.0;
        inc += 4.1; if( inc >= 5.0 ){ inc -= 10.0; }
        //IO_Set(1,inc);
        //IO_Set(2,inc);
        //IO_Set(3,inc);
        //IO_Set(1,inc);
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
    while(1){;;}
}
