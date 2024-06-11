#include "main.h"

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
// #include "usbd/usbd_cdc_interface.h" // CDC_main_init()
#include "usbh/usbh_main.h"
#include "lib/bootloader.h" // bootloader_enter(), bootloader_restart()
#include "lib/flash.h" // Flash_clear_user_script()
#include "stm32f7xx_it.h" // CPU_count;

// new USB composite device setup
#include "lib/usb_otg.h"
#include "usb_device.h"
#include "usbd_midi_if.h"

#include "lib/midi.h"
// #include "lib/mhost.h"
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
    Debug_USART_Init(); // ignored in TRACE mode
    // User-readable status led
    status_led_init();
    // status_led_fast(LED_SLOW); // slow blink until USB connection goes live
    // status_led_set(1); // set status to ON to show sign of life straight away

    printf("\n\nhi from crow!\n\r");
    // U_PrintNow();

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
    // CDC_clear_buffers();

    // new usb composite device
    MX_USB_OTG_FS_PCD_Init();
    // printf("f\n\r");
    // U_PrintNow();
    MX_USB_DEVICE_Init();

    // MIDI Host
    // MHost_Init();
    USBHost_Init();

    // i2c_hw_pullups_init(); // enable GPIO for v1.1 hardware pullups
    // ii_init( II_CROW );
    // Random_Init();

    // REPL_init( Lua_Init() );

    // REPL_print_script_name();
    // Lua_crowbegin();

// TODO startup animation
    // here we run the power sequence to spread out current spikes when enabling
    // setup the leds first so we can draw a nice animation while things get going
    // mostly just doing this to get a predictable state before enabling USB Host
    // HAL_Delay(1000);

    // Enable USB Host power
    // MHost_Power(1);



    // uint32_t last_tick = HAL_GetTick();
    int saw = 0;
    int g_state = 0;
    // int counter = 0;
    int once = 1;
    while(1){
        CPU_count++;

        saw++;
        saw &= 0x3ffff;
        if(saw == 0x1ffff){
            Debug_Pin_Set(1, g_state);
            g_state ^= 1;

            uint8_t midi_msg[3] = {0x90, 0x3c, 0x64};
            MIDI_transmit(&midi, midi_msg, 3);

            if(once){
                once = 0;

                // char crow_msg[32];
                // sprintf(crow_msg, "output[1].volts = %i\n\r", counter++);
                // sprintf(crow_msg, "^^v\n\r", counter++);
                // if(counter > 10) counter = 0;
                // USBHost_Send(crow_msg, strlen(crow_msg)+1);
            }
            // char crow_msg[64];
            // snprintf(crow_msg, 64, "print(time())\n\r");
            // char* crow_msg = "^^v\n\r";
            char* crow_msg = "print('hi')\n\r";
            USBHost_Send((unsigned char*)crow_msg, strlen(crow_msg));
            // Caw_printf("hi\n\r");

            // the midiparser lib probably wraps this
            // 144 60 127 - turn ON note #60 on MIDI channel 1 with a velocity of 127
            uint8_t cable = 0;
            uint8_t code = 0x9; // note-on message (see usb-midi pdf doc)
            uint8_t message = 0x9; // note-on
            uint8_t channel = 0;
            uint8_t note = 60;
            uint8_t velocity = 127;

            uint8_t reportBuffer[4] = {
              // cable - represents physical/virtual port number (0 - 15) of the device
              // code - in general cases is equal to midi message
              (cable << 4) | code,
              (message << 4) | channel,
              note,
              velocity,
            };
            // while (USBD_MIDI_GetState() != MIDI_IDLE) {};
            USBD_MIDI_SendReport(reportBuffer, 4);
            Caw_printf(".\n\r");
        }

        U_PrintNow();
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

        if( USBHost_BG_Task() ){ // data is ready to be used
            uint8_t* buf;
            size_t len = USBHost_Get_Received(&buf);
            len = len > 100 ? 100 : len;
            char crow_msg[100];
            /*
            for(int i=0; i<len; i++){
                sprintf(&crow_msg[i*3], "%2x ", buf[i]);
                // we overwrite each NULL char with next set
                // last call leaves trailing NULL
            }
            */
            // sprintf(&crow_msg[len*3], "\n\r");
            snprintf(crow_msg, len, (char*)buf);
            Caw_printf("C:%s\n\r", crow_msg);
        }
    }
}

void Error_Handler(void)
{
    printf("Error Handler\n");
    U_PrintNow();
    while(1){;;}
}
