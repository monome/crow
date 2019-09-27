#pragma once

#define RNG_IRQPriority 4
#define RNG_IRQSubPriority 1


void Random_Init(void);
void Random_DeInit(void);

float Random_Float(void);
int Random_Int(int lower, int upper);
void Random_Update(void);
