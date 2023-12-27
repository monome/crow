#pragma once

#include <stm32f7xx.h>

void TP_Init(void);

/////////////////////////////////
// static control
void TP_power12(int state);
void TP_hub(int state);
void TP_dout(int index, int state); // 0-5
void TP_debug_led(int index, int state);

void TP_dac_mux_1(int chan);
void TP_dac_mux_2(int chan);
void TP_jack_direction(int select);

void TP_adc_mux_1(int chan); // controlled by ADC driver

void TP_dac(int chan, float value);

/////////////////////////////////
// read on demand

// 0b00000 = MG
// 0b00001 = TS
// 0b00010 = CM
// 0b00011 = JF
// 0b11111 = INVALID: no device detected
int TP_get_module_id(void);

/////////////////////////////////
// read from callback
int TP_cherry_state(int index);

/////////////////////////////////
// read dynamically
