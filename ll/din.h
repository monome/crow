#pragma once

typedef enum{
    DIN_RESET,
    DIN_DOWN,
    DIN_2UP
} DINx;

void din_init(void);

int din_get(DINx ch);
