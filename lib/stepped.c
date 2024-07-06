#include "stepped.h"

int steps = 12;
int current_step = 0;
int reset = 0;
int twoup = 0;
int down = 0;

int vsteps[12] = {0};

void stepped_init(void){
    current_step = 0;
    reset = 0;
    twoup = 0;
    down = 0;

    float total_real_volts = (2.5f * 120.f / 24.9f);
    float offset_real_volts = (2.5f * 120.f / 49.9f);

    float one_volt = (float)(0xfff) / total_real_volts; // ~340 lsbs per volt
    float half_range = offset_real_volts * one_volt;

    for(int i=0; i<6; i++){
        vsteps[i] = (int)(half_range - one_volt * i);
    }
    for(int i=6; i<12; i++){
        vsteps[i] = (int)(half_range - one_volt * (i-12));
    }
}

void stepped(float _steps, int _reset, int _twoup, int _down){
    // perform reset first so we can wrap & then advance on simultaneous triggers
    if(_reset != reset){
        reset = _reset;
        if(reset == 1){
            current_step = 0;
        }
    }

    // adjust playhead
    if(_twoup != twoup){
        twoup = _twoup;
        if(twoup == 1){
            current_step += 2;
        }
    }
    if(_down != down){
        down = _down;
        if(down == 1){
            current_step -= 1;
        }
    }

    // wrap to steps
    // don't use modulo to correctly handle negative values
    // could just add steps, and then modulo for simpler solution
    steps = 1 + (int)(_steps*11.999f); // [1,12)
    while(current_step < 0){
        current_step += steps;
    }
    while(current_step >= steps){
        current_step -= steps;
    }
}

int stepped_ix(void){
    return current_step;
}

int stepped_get(void){
    // perform scaling
    return vsteps[current_step];
}
