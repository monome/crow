#pragma once

// setup data structures
void sfold_init(int channels);

// apply control settings
// affects *all channels*
void sfold_set_fold(float f);
void sfold_set_rotate(float r);
void sfold_set_density(float d);
void sfold_set_offset(float o);
void sfold_set_id(float i);

// separate run function per channel
float sfold(int channel);
