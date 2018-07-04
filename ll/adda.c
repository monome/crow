#include "adda.h"

#include "debug_pin.h"
#include "ads131.h"
#include "dac8565.h"

uint16_t ADDA_Init( void )
{
    ADC_Init( ADDA_BLOCK_SIZE
            , ADDA_ADC_CHAN_COUNT
            );
    DAC_Init( ADDA_BLOCK_SIZE
            , ADDA_DAC_CHAN_COUNT
            );
    return ADDA_BLOCK_SIZE;
}

void ADDA_Start( void )
{
    DAC_Start();
}

void ADDA_BlockProcess( uint32_t* dac_pickle_ptr )
{
    IO_block_t b = { .size = ADDA_BLOCK_SIZE };
    Debug_Pin_Set(1);

    ADC_UnpickleBlock( b.in[0]
                     , ADDA_BLOCK_SIZE
                     );
    IO_BlockProcess( &b );
    DAC_PickleBlock( dac_pickle_ptr
                   , b.out[0]
                   , ADDA_BLOCK_SIZE
                   );
    Debug_Pin_Set(0);
}

__weak IO_block_t* IO_BlockProcess( IO_block_t* b )
{
    static float count = 1.0;
    for( uint16_t i=0; i<(b->size); i++ ){
        b->out[0][i] = b->in[0][i];
        b->out[1][i] = b->in[1][i];
        b->out[2][i] = 2.0;
        b->out[3][i] = 3.0;

        count += 0.9; if(count>=3.0){ count -= 6.0; }
    }
    return b;
}
