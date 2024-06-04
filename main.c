#include "ll/system.h"
#include "ll/debug_pin.h"
#include "ll/debug_usart.h"
#include "ll/status_led.h"
#include "syscalls.c" // printf() redirection
#include "lib/io.h"
#include "lib/events.h"
#include "ll/timers.h"
#include "lib/metro.h"
// #include "lib/clock.h"
#include "lib/caw.h"
// #include "lib/ii.h"
#include "ll/i2c_pullups.h" // i2c_hw_pullups_init
#include "ll/random.h"
#include "lib/lualink.h"
// #include "lib/repl.h"
#include "usbd/usbd_cdc_interface.h" // CDC_main_init()
#include "lib/bootloader.h" // bootloader_enter(), bootloader_restart()
#include "lib/flash.h" // Flash_clear_user_script()
#include "stm32f7xx_it.h" // CPU_count;

#include "lib/midi.h"
#include "ll/uart.h"

static Uart uart_rx;
static Uart uart_tx;
static Midi midi;

int main(void)
{
    system_init();

    // Debugging
    Debug_Pin_Init();
    Debug_Pin_Set(0,1);
    // Debug_USART_Init(); // ignored in TRACE mode
    // User-readable status led
    status_led_init();
    // status_led_fast(LED_SLOW); // slow blink until USB connection goes live
    // status_led_set(1); // set status to ON to show sign of life straight away

    printf("\n\nhi from crow!\n\r");

    // MIDI
    // pass initialized uart handlers to midi system
    MIDI_init(&midi, UART_init(&uart_tx, UART_Tx, UART8, "E1"));
    // setup midi reception uart
    UART_init(&uart_rx, UART_Rx, UART5, "B8");
    // redirect uart interrupts to midi system
    UART_set_callback(&uart_rx, MIDI_get_callback(&midi));

    // Drivers
    int max_timers = Timer_Init();
    // IO_Init( max_timers-2 ); // use second-last timer
    // IO_Start(); // must start IO before running lua init() script
    // events_init();
    // Metro_Init( max_timers-2 ); // reserve 2 timers for USB & ADC
    // clock_init( 100 ); // TODO how to pass it the timer?
    Caw_Init( max_timers-1 ); // use last timer
    CDC_clear_buffers();

    // i2c_hw_pullups_init(); // enable GPIO for v1.1 hardware pullups
    // ii_init( II_CROW );
    // Random_Init();

    // REPL_init( Lua_Init() );

    // REPL_print_script_name();
    // Lua_crowbegin();

    uint32_t last_tick = HAL_GetTick();
    int saw = 0;
    int g_state = 0;
    int counter = 0;
    while(1){
        CPU_count++;

        saw++;
        saw &= 0xfffff;
        if(saw == 0x7ffff){
            Debug_Pin_Set(1, g_state);
            g_state ^= 1;
            // Caw_printf("hi\n\r");
            // Caw_printf("%i\n\r",counter++);
            uint8_t midi_msg[3] = {0x90, 0x3c, 0x64};
            MIDI_transmit(&midi, midi_msg, 3);
        }

        // U_PrintNow();
        Caw_try_receive(); // something is broken in the receiver :/
            // prob something to do with the different chip?
        // switch( Caw_try_receive() ){ // true on pressing 'enter'
        //     case C_repl:        REPL_eval( Caw_get_read()
        //                                  , Caw_get_read_len()
        //                                  , Caw_send_luaerror
        //                                  ); break;
        //     case C_boot:        bootloader_enter(); break;
        //     case C_startupload: REPL_begin_upload(); break;
        //     case C_endupload:   REPL_upload(0); break;
        //     case C_flashupload: REPL_upload(1); break;
        //     case C_restart:     bootloader_restart(); break;
        //     case C_print:       REPL_print_script(); break;
        //     case C_version:     system_print_version(); break;
        //     case C_identity:    system_print_identity(); break;
        //     case C_killlua:     REPL_reset(); break;
        //     case C_flashclear:  REPL_clear_script(); break;
        //     case C_loadFirst:   REPL_default_script(); break;
        //     default: break; // 'C_none' does nothing
        // }
        // Random_Update();

        // uint32_t time_now = HAL_GetTick(); // for running a 1ms-interval tick
        // if( last_tick != time_now ){ // called on 1ms interval
        //     last_tick = time_now;
        //     // clock_update(time_now);
        //     status_led_tick(time_now);
        // }

        // event_next(); // check/execute single event
        // ii_leader_process();
        Caw_send_queued();
    }
}
