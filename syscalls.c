#include <stm32f7xx.h>
#include "ll/debug_usart.h"

// TODO implement a str_buffer here to avoid overflowing the TRACE pin?

int _write(int file, char* data, int len)
{
#ifdef TRACE
    int count = len;
    while(count--){
        fputc((uint32_t)*data++, (FILE*)file);
    }
#else // TRACE using USART to debug
    U_Print(data,len); // enqueues a string
#endif // TRACE
    return len;
}

// use an #ifdef to choose ITM or USART (CLI flag?)
int fputc(int ch, FILE *f)
{
    f = f;
    ITM_SendChar( (uint32_t)ch );
    return ch;
}
