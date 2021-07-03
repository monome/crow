#pragma once

#define RNG_IRQPriority 4
#define RNG_IRQSubPriority 1


// Call Init to activate hardware peripheral
void Random_Init(void);
void Random_DeInit(void);

// Get a single random value with these 2 fns
float Random_Float(void);
int Random_Int(int lower, int upper);

// Place Update in the main loop to keep random buffer full
void Random_Update(void);
