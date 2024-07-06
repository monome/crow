#include "adda.h"

#include "adc.h"
#include "dac108.h"
#include "../lib/sfold.h"

static uint32_t stepper[16] = {0};

static uint32_t vals[4] = {0}; // direct set values for non-folds

static void write_to_dac_buffer(uint32_t* dest, int channel, int sample, int src);

void ADDA_BlockProcess(uint32_t* dest, int bsize){
    uint32_t samp = 0;

    // smooth & stepped modulation
    for(int i=0; i<bsize; i++){ // samples
        for(int j=0; j<4; j++){ // channels
            // samp is a 12bit value, right-justified in u32
            samp = vals[j]; // override with set value

            write_to_dac_buffer(dest, j, i, samp);
        }
    }

    const float i12f = 1.f / (float)0xfff;

    // TODO move this inside the bsize loop once we have per-sample ADC
    sfold_set_fold(i12f * (float)ADC_get(ADC_Fold));
    sfold_set_rotate(i12f * (float)ADC_get(ADC_Rotate));
    sfold_set_density(i12f * (float)ADC_get(ADC_Density));
    sfold_set_offset(i12f * (float)ADC_get(ADC_Offset));
    sfold_set_id(i12f * (float)ADC_get(ADC_ID));

    for(int i=0; i<bsize; i++){ // samples
        for(int j=4; j<16; j++){ // channels
            // samp is a 12bit value, right-justified in u32
            uint32_t samp = stepper[j];
            stepper[j] += j+1;
            stepper[j] &= 0xfff;

            float sf = sfold(j-4);

            write_to_dac_buffer(dest, DAC_get_channel_id(j-4)
                               , i, (int)(sf*(float)0xfff));
            // write_to_dac_buffer(dest, j, i, samp);
        }
    }
}

void ADDA_set_val(int ch, uint32_t val){
    if((ch>=0) && (ch<4)){
        vals[ch] = val & 0xfff;
    }    
}

// dest* is an array of u32 sample slots
// output should be organized: 0,8,1,9,2,10 .. 7,15
// with bsize of these in series

// can vectorize this later to improve performance
// really just need to unroll the loop
// honestly this is probably neglible on the f722 chip
// really just wanted to abstract this away as it was confusing to get right
static void write_to_dac_buffer(uint32_t* dest, int channel, int sample, int src){
    int jj = (channel & 7) << 1; // bottom 3 bits, x2. 0/8 follow each other
    int jx = channel >> 3; // 1 if an upper set. upper 8 are 1 step ahead of lower
    int ix = sample * 16; // for each sample, step forward by 16 samples (there are 16 channels)
    dest[jj+jx+ix] = (jj<<11) | (src & 0xfff); // hard cut values to 12bit range
}
