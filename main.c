#include "ll/system.h"
#include "ll/debug_pin.h"
#include "ll/debug_usart.h"
#include "syscalls.c" // printf() redirection
#include "lib/io.h"
#include "lib/events.h"
#include "lib/metro.h"
#include "lib/caw.h"
#include "lib/ii.h"
#include "ll/random.h"
#include "lib/lualink.h"
#include "lib/repl.h"
#include "usbd/usbd_cdc_interface.h" // CDC_main_init()
#include "lib/bootloader.h" // bootloader_enter(), bootloader_restart()
#include "lib/flash.h" // Flash_clear_user_script()

int main(void)
{
    system_init();

    // Debugging
    Debug_Pin_Init();
    Debug_USART_Init(); // ignored in TRACE mode

    printf("\n\nhi from crow!\n");

    // Drivers
    IO_Init();
    IO_Start(); // must start IO before running lua init() script
    events_init();
    Metro_Init();
    Caw_Init();
    CDC_clear_buffers();
    II_init( II_CROW );
    Random_Init();

    REPL_init( Lua_Init() );

    Lua_crowbegin();


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
            case C_version:    system_print_version(); break;
            case C_identity:   system_print_identity(); break;
            default: break; // 'C_none' does nothing
        }
        Random_Update();
        // check/execute single event
        event_t e;
        if( event_next(&e) ){
            (*app_event_handlers[e.type])(&e);
        }
    }
}
