#pragma once

#include <stm32f7xx.h>

void ADDA_BlockProcess(uint32_t* dest, int bsize);
void ADDA_set_val(int ch, uint32_t val);
