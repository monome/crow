#pragma once

void lights_init(void);

void lights_all(int state);
void lights_set(int ch, int state);
void lights_xset(int ch); // exclusive set
void lights_range(int min, int max, int state);

// TODO PWM? so we can do a dim range?
