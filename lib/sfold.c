#include "sfold.h"

#include <stdint.h>
#include <math.h>


static float fold, rotate, density, offset, id = 0;
static float rotated, iid = 0; // OPTIMIZATION
static float _ch = 1.f; // total count of channels to span across
static float _ich = 1.f; // inverse of _ch

inline static float _xfade( float a, float b, float c );
static float folder(float in, float coeff);
static float densify(float v, float density);
    

// setup data structures
void sfold_init(int channels){
    _ch = (float)channels;
    _ich = 1.f / _ch;

    fold = 0.f;
    rotate = 0.f;
    density = 0.f;
    offset = 0.f;
    id = 0.f;
}

// apply control settings
// affects *all channels*
// call these every sample!
// do float-ing upsampling outside of this function

// (0..1)
void sfold_set_fold(float f){
    fold = f;
}
// (0..1)
void sfold_set_rotate(float r){
    rotate = r;
    rotated = (rotate - 0.5f) * _ch; // allows rotate to perform 360
}
// (0..1)
void sfold_set_density(float d){
    density = d;
}
// (0..1)
void sfold_set_offset(float o){
    offset = o;
}
// (0..1)
void sfold_set_id(float i){
    id = i;
    iid = 1.f - id;
}

// separate run function per channel
// returns 0..1
static float phasor = 0.f;
static float pinc = 100.f / (2.f * 6000.f); // are we running twice per frame?
float sfold(int channel){
    // OFFSET
    float retval = offset;

    // tmp: sawtooth oscillator
    // float retval = phasor;
    // if(channel == 0){
    //     phasor += pinc; 
    //     if(phasor>=1.f) phasor -= 1.f;
    // }


    // ROTATE
    // create rotated_index by adding ROTATE to channel index
    // float rix = (float)channel + ((rotate - 0.5f) * _ch); // allows rotate to perform 360
    float rix = (float)channel + rotated; // allows rotate to perform 360
    // wrap into range of 0 to channel count
    while(rix < 0.f) rix += _ch;
    while(rix >= _ch) rix -= _ch;
    rix *= _ich; // scale down to 0,1
    rix -= 0.5f; // centre around 0 (-0.5, 0.5)
    rix *= 6.f; // scale up to +/-3

    // ID
    // apply "id" (modulo) to OFFSET by combining CV and rotated_index
    // retval += rix * (1.f-id);
    retval += rix * iid;
    while(retval < 0.f) retval += 1.f;
    while(retval >= 1.f) retval -= 1.f;


    // FOLD
    // apply "wavefold" (use just friends implementation)
    retval = folder(2.f*retval - 1.f, 2.f * (1.f-fold));
    retval /= 2.f;
    retval += 0.5; // normalize to 0..1

    // DENSITY
    // apply "density" (a simple exp/lin/log linear transformation)
    retval = densify(retval, density);

    return retval;
}

// in (0 .. 1) if not using _sin
// coeff (0 .. 2)
static float folder(float in, float coeff){
    float gain = 1.2 * coeff * coeff; // exponential (^2) application

    // apply the scaled coeff as a gain constant
    in *= gain + 1.0; // 1-6x gain
    // in += gain * 0.5; // TODO fine tune this value to acheive ideal sound


    int win2 = ((int)(in + 8.f)) - 8;
    switch(win2){
        case -7: case -6: return -6.f - in;
        case -5: case -4: return in + 4.f;
        case -3: case -2: return -2.f - in;
        case -1: case  0: return in;
        case  1: case  2: return 2.f - in;
        case  3: case  4: return in - 4.f;
        case  5: case  6: return 6.f - in;
        case  7: case  8: return in - 8.f;
        default:          return 0.f; // shouldn't happen
    }

    // perform the wavefold
    // int win2 = ((int)(in + 7.f)) >> 1;
    // float offset = (float)((win2 << 1) - 6); // multiply by 2 then subtract 6
    // // if LSB is 1 we choose to invert
    // return (win2 & 1) ? (in - offset) : (offset - in) ;
}

inline static float _xfade( float a, float b, float c ){
    return (a + c*(b-a));
}

// LUTs for shaping for easier customization & much faster operation
#include "lin2expo.h"
#include "lin2logo.h"
// retval & density: 0 .. 1
// density is a crossfade from expo -> linear -> log
// returns a 0..1 float which distorts the midpoint of the waveform up/down
static float densify(float v, float density){
    if(density<0.5f){ // less dense. use exponential
        float vv = v * 255.f; // convert 0,1 to 0,255
        int left = (int)vv;
        float c = vv - (float)left;
        float expo = _xfade(lut_expo_256[left], lut_expo_256[left+1], c);

        // float expo = powf(2.f, 10.f * (v - 1.f));
        return _xfade(expo, v, density * 2.f); // at 1, we want linear
    } else if(density > 0.5f){ // more dense. use logarithmic
        float vv = v * 255.f; // convert 0,1 to 0,255
        int left = (int)vv;
        float c = vv - (float)left;
        float logo = _xfade(lut_logo_256[left], lut_logo_256[left+1], c);

        // float log = 1.f - powf(2.f, -10.f * v);
        return _xfade(v, logo, (density * 2.f)-1.f); // at 0, we want linear
    } else {
        return v;
    }
}
