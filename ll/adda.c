#include "adda.h"

// dest is an array of u32 sample slots
// output should be organized: 0,8,1,9,2,10 .. 7,15
// with bsize of these in series
static uint32_t stepper[16] = {0};

static uint32_t vals[6] = {0};

void ADDA_BlockProcess(uint32_t* dest, int bsize)
{
    // IO_block_t b = { .size = ADDA_BLOCK_SIZE };
    // ADC_UnpickleBlock( b.in[0]
    //                  , ADDA_BLOCK_SIZE
    //                  );
    // IO_BlockProcess( &b );
    // DAC_PickleBlock( dac_pickle_ptr
    //                , b.out[0]
    //                , ADDA_BLOCK_SIZE
    //                );
    for(int i=0; i<bsize; i++){ // samples
        for(int j=0; j<16; j++){ // channels
            // samp is a 12bit value, right-justified in u32
            uint32_t samp = stepper[j];
            if(j<6){
                samp = vals[j]; // override with set value
            } else {
                stepper[j] += j+1;
                stepper[j] &= 0xfff;
            }

            int jj = (j&7)<<1; // bottom 3 bits, x2. 0/8 follow each other
            int jx = j>>3; // 1 if an upper set. upper 8 are 1 step ahead of lower
            int ix = i*16; // for each sample, step forward by 16 samples (there are 16 channels)
            dest[jj+jx+ix] = (jj<<11) | (samp & 0xfff); // hard cut values to 12bit range
        }
    }
}

void ADDA_set_val(int ch, uint32_t val){
    if((ch>=0) && (ch<6)){
        vals[ch] = val & 0xfff;
    }    
}
