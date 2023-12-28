#include "tp.h"

static void init_power12(void);
static void init_hub(void);
static void init_dout(void);
static void init_module_id(void);
static void init_debug_leds(void);
static void init_cherry(void);

static void init_dacmux1(void);
static void init_dacmux2(void);
static void init_jack_dir(void);
static void init_adcmux(void);

static GPIO_InitTypeDef g;
void TP_Init(void){
    // enable peripheral clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    init_power12();
    init_hub();

    init_dout();
    init_module_id();
    init_debug_leds();
    init_cherry();

    init_dacmux1();
    init_dacmux2();
    init_jack_dir();
    init_adcmux();


    // default states
    TP_power12(0); // DUT power is disabled
    TP_hub(1); // enable to make sure we're not competing with hw pullup
        // can change this in future after uC firmware is complete
        // uC chooses when to activate the HUB (prob just want always on)
    for(int i=0; i<5; i++){
        TP_dout(i, 0); // disable digital outputs
    }
    for(int i=0; i<2; i++){
        TP_debug_led(i, 0); // disable leds
    }
    // disable all MUXes to make sure DUT is isolated when power goes high
    TP_dac_mux_1(-1);
    TP_dac_mux_2(-1);
    TP_jack_direction(-1);
    TP_adc_mux_1(-1);



    // ADCs
    // ADC1_In0: A0
    // ADC2_In1: A1
    // ADC3_In2: A2
    // ADC1_In3: A3 // POWER
    // ADC2_In4: A4

    // DAC
    // DAC108_FS / SAI1_FS_A = E4
    // DAC108_SCK / SAI1_SCK_A = E5
    // DAC108_SD_A / SAI1_SD_A = E6

    // UNUSED
    // might activate in future
    // Cherry LEDs (unused): C6, C7
    // DUT UART: Uart 5, Rx=B8, Tx=B9
}

// POWER-ENABLE: E2
static void init_power12(void){
    g.Pin   = GPIO_PIN_2;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOE, &g);
}
void TP_power12(int state){
    state = !!state;
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, state);
}

// HUB_RESET: E0
static void init_hub(void){
    g.Pin   = GPIO_PIN_0;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOE, &g);
}
void TP_hub(int state){
    state = !!state;
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, state);
}

// 6x DOUT (led control on DUT): D8, B15, B14, B13, B12, E15
static void init_dout(void){
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_LOW;

    g.Pin   = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOB, &g);
    g.Pin   = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOD, &g);
    g.Pin   = GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &g);
}
void TP_dout(int index, int state){
    state = !!state; // force 0/1
    if(index >= 0 && index < 6){
        switch(index){
            case 0: HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, state); break;
            case 1: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, state); break;
            case 2: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, state); break;
            case 3: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, state); break;
            case 4: HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, state); break;
            case 5: HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, state); break;
        }
    }
}

// 5x DIN (DUT module id): D13, D12, D11, D10, D9
static void init_module_id(void){
    g.Mode  = GPIO_MODE_INPUT;
    g.Pull  = GPIO_PULLUP; // ie. floating is HIGH
    g.Pin   = GPIO_PIN_13 | GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOD, &g);
}
int TP_get_module_id(void){
    uint8_t id = 0;
    id |= HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_13);
    id |= HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_12) << 1;
    id |= HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_11) << 2;
    id |= HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_10) << 3;
    id |= HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_9) << 4;
    return (int)id;
}

// Debug LEDs (1&2 are already used): C9, C8
static void init_debug_leds(void){
    g.Pin   = GPIO_PIN_9 | GPIO_PIN_8;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC, &g);
}
void TP_debug_led(int index, int state){
    state = !!state;
    if(index == 0){
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, state);
    } else if(index == 1){
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, state);
    }
}

// Cherry switches (2): D14, D15
static void init_cherry(void){
    g.Mode  = GPIO_MODE_INPUT;
    g.Pull  = GPIO_PULLUP; // ie. floating is HIGH
    g.Pin   = GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOD, &g);
}
int TP_cherry_state(int index){
    // NOTE: we invert output states due to GND being "active"
    if(index == 0){
        return !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14);
    } else if(index == 1){
        return !HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_15);
    }
    return -1; // invalid index
}

    // pins in order of EN, S0, S1, S2
    // DACMUX1 output (4): A6, A7, C5, C4
static void init_dacmux1(void){
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FAST;

    g.Pin   = GPIO_PIN_6 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOA, &g);
    g.Pin   = GPIO_PIN_5 | GPIO_PIN_4;
    HAL_GPIO_Init(GPIOC, &g);
}
void TP_dac_mux_1(int chan){
    if(chan < 0){
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0); // disable
    } else if(chan < 8){
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1); // enable
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, chan & 0b1);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, (chan & 0b10)>>1);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, (chan & 0b100)>>2);
    }
}

// DACMUX2 output (4): B1, B2, E8, E7
static void init_dacmux2(void){
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FAST;

    g.Pin   = GPIO_PIN_1 | GPIO_PIN_2;
    HAL_GPIO_Init(GPIOB, &g);
    g.Pin   = GPIO_PIN_8 | GPIO_PIN_7;
    HAL_GPIO_Init(GPIOE, &g);
}
void TP_dac_mux_2(int chan){
    if(chan < 0){
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 0); // disable
    } else if(chan < 8){
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, 1); // enable
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, chan & 0b1);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, (chan & 0b10)>>1);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, (chan & 0b100)>>2);
    }
}

// JACKMUX output (2): C14, C13
static void init_jack_dir(void){
    g.Pin   = GPIO_PIN_13 | GPIO_PIN_14;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOC, &g);
}
void TP_jack_direction(int select){
    switch(select){
        case -1:
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0);
            break;
        case 0:
            TP_dac_mux_2(0); // Force DAC to correct channel
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
            break;
        case 1:
            // TODO force ADC to switch (or maybe we're just scanning so it doesn't matter?)
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
            break;
        default: break;
    }
}

// ADCMUX output (4): D3, D4, D6, D5
static void init_adcmux(void){
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FAST;
    g.Pin   = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOD, &g);
}
void TP_adc_mux_1(int chan){
    if(chan < 0){
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, 0); // disable
    } else if(chan < 8){
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, 1); // enable
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, chan & 0b1);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, (chan & 0b10)>>1);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5, (chan & 0b100)>>2);
    }
}

#include "dac108.h"
void TP_dac(int chan, float value){
    dac108_send(chan-1, value);
}

#include "adc.h"
static const int adc_lookup[] = {
    4, // adc0 = jack
    36, // adc1
    31,
    26,
    21,
    17, // 5
    12,
    7,
    2,
    37,
    32, // 10
    27,
    22,
    18,
    13,
    8, // 15
    3,
    38,
    33,
    28,
    23, // 20
    19,
    14,
    9,
    4,
    39, // 25
    34,
    29,
    24, // adc28
// TODO allow string lookup for these
    0, // 29 = +i_DUT // 1.1107
    5, // 30 = -i_DUT // 0.550
    10, // 31 = +i_PTC // 1.107
    15, // 32 = -i_PTC // 0.183
    20, // 33 = +12V // 2.4
    25 // 34 = -12V // 2.42
};
float TP_adc(int chan){
    // TODO map requested channel to memory-channel (many holes in the adc array)
    if(chan<0 || chan > 34){
        return -6.66; // BAD CHANNEL
    } else if(chan >= 29){ // power channel
        float a = ADC_get(adc_lookup[chan]); // lua is 1-based
        switch(chan){
            case 29: case 30: // i_DUT // 1V = 100mA
                a = 100 * 3*(a/4095.0); // convert to real voltage, then 100x to mA
                break;
            case 31: case 32: // i_PTC -> actually trying to return measured resistance of PTC
                a = (3*(a/4095.0) / 3.69697) // calculate real v-drop across PTC
                        / (0.1 * 3*((a-2)/4095.0)); // divide it by measured current (in amps)
                break;
            case 33: case 34: // +/-12V (absolute)
                a = 5 * 3*(a/4095.0); // convert to real voltage, then mult by 5
                break;
            default: break;
        }
        // TODO convert to human-readable form
        return a;
    } else { // regular channel
        float a = ADC_get(adc_lookup[chan]);
        a = 3.0 * a/4095.0; // actual measured voltage on pin
        a -= 1.5; // convert to +/- 1v5
        a *= -6.666667; // convert to +/- 10V (and invert)
        return a; // lua is 1-based
    }
}

/* PIN MAPPINGS for ADC buffer

ACTUALLY!! must subtract one from all of these indices
the lookup table above is corrected for this

1 +CURRENT_DUT (internal TP + DUT consumption)
6 -CURRENT_DUT (internal TP + DUT consumption)
11 +CURRENT_PTC (Vdrop across PTC)
16 -CURRENT_PTC (Vdrop across PTC)
21 +12V (+2v4 scaled) -> actual output voltage (check for droop)
26 -12V (+2v4 scaled) -> actual output voltage (check for droop)

22 ADC4
27 ADC3
32 ADC2
37 ADC1

3 ADC8
8 ADC7
13 ADC6
18 ADC5
23 ADC12
28 ADC11
33 ADC10
38 ADC9

4 ADC16
9 ADC15
14 ADC14
19 ADC13
24 ADC20
29 ADC19
34 ADC18
39 ADC17

5 ADC24 / IN JACK
10 ADC23
15 ADC22
20 ADC21
25 ADC28
30 ADC27
35 ADC26
40 ADC25

*/
