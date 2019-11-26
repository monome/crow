#pragma once

float shapes_sin( float in );
float shapes_log( float in );
float shapes_exp( float in );
float shapes_step_now( float in );
float shapes_step_wait( float in );
float shapes_ease_in_back( float in );
float shapes_ease_out_back( float in );
float shapes_ease_out_rebound( float in );

float* shapes_v_sin( float* in, int size );
float* shapes_v_log( float* in, int size );
float* shapes_v_exp( float* in, int size );
