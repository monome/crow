#include "ll/system.h"
// #include "ll/debug_pin.h"
// #include "ll/debug_usart.h"
#include "ll/status_led.h"
#include "syscalls.c" // printf() redirection
// #include "lib/io.h"
// #include "lib/events.h"
#include "ll/timers.h"
// #include "lib/metro.h"
#include "lib/clock.h"
#include "lib/caw.h"
// #include "lib/ii.h"
// #include "ll/i2c_pullups.h" // i2c_hw_pullups_init
// #include "ll/random.h"
#include "usbd/usbd_cdc_interface.h" // CDC_main_init()
#include "stm32f7xx_it.h" // CPU_count;
#include "ll/lights.h"
#include "ll/din.h"
#include "ll/adc.h"
#include "ll/dac108.h"
#include "ll/adda.h"
#include "lib/stepped.h"
#include "lib/sfold.h"

/*
fold, A7, 1_in7
id, B0, 1_in8
offset, B1, 1_in9
density, C4, 2_in14
steps, A5, 2_in5
rotate, A6, 2_in6

MOSI, A0, SAI2_SD_B, AF10
SCK, A2, SAI2_SCK_B, AF8
!SYNC, C0, SAI2_FS_B, AF8
*/


int main(void){
    system_init();

    // Debugging
    // Debug_Pin_Init();
    Debug_USART_Init(); // ignored in TRACE mode
    // User-readable status led
    status_led_init();
    status_led_fast(LED_SLOW); // slow blink until USB connection goes live
    status_led_set(1); // set status to ON to show sign of life straight away

    printf("\n\nhi from parafocus!\n");

    lights_init();
    lights_all(0);

    din_init();

    ADC_Init();

    // Drivers
    int max_timers = Timer_Init();
    // IO_Init( max_timers-2 ); // use second-last timer
    // IO_Start(); // must start IO before running lua init() script
    // Metro_Init( max_timers-2 ); // reserve 2 timers for USB & ADC
    clock_init( 100 ); // TODO how to pass it the timer?
    Caw_Init( max_timers-1 ); // use last timer
    CDC_clear_buffers();

    // i2c_hw_pullups_init(); // enable GPIO for v1.1 hardware pullups
    // ii_init( II_CROW );
    // Random_Init();

    stepped_init();
    sfold_init(12);

    // DAC_Init(32, 16); // 32 samples per block, 16 channels
    DAC_Init(1, 16); // DISABLE BLOCK PROCESSING, single sample! minimal latency
    DAC_Start();

    uint32_t last_tick = HAL_GetTick();
    int counter = 100;
    uint8_t state = 0;
    int l_count = 0;
    while(1){
        CPU_count++;
        U_PrintNow();
        Caw_try_receive(); // JUST DROP RX'D VALS

        // Random_Update();
        uint32_t time_now = HAL_GetTick(); // for running a 1ms-interval tick
        if( last_tick != time_now ){ // called on 1ms interval
            last_tick = time_now;
            clock_update(time_now);
            // status_led_tick(time_now);
            counter--;
            if(counter<=0){
                counter = 100;
                state ^= 1;
                status_led_set(state);
                l_count++;
                if(l_count >= 12) l_count = 0;

                int a = ADC_get(4); // raw 0~4095 value
                a *= 12; // scale up to 12*4096
                a >>= 12; // divide by 4096
                lights_xset(stepped_ix());
                // Caw_printf("%i\n\r",a);
                // Caw_printf("%i\n\r",ADC_get_count());
            }
            for(int i=0; i<6; i++){
                // ADDA_set_val(i, ADC_get(i));
            }
        }

        stepped(1.f, din_get(DIN_RESET), din_get(DIN_2UP), din_get(DIN_DOWN));
        ADDA_set_val(0, stepped_get());
        ADDA_set_val(1, 0x0); // set fine tune to zero position
        // NOTE: chan 0 is main stepped cv, chan1 is fine tune

        // lights_set(0, din_get(DIN_RESET));
        // lights_set(1, din_get(DIN_DOWN));
        // lights_set(2, din_get(DIN_2UP));
        // ii_leader_process();
        Caw_send_queued();
    }
}
