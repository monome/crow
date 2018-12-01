typedef enum{ S_linear
            , S_square
            , S_log
            , S_sine
} S_shape_t;

// Lookup tables
int   LUT_sine_sz = 4;
float LUT_sine[LUT_sine_sz+1] = {0.0, 0.125, 0.25, 0.5, 1.0};
int   LUT_log_sz = 4;
float LUT_log[LUT_log_sz+1] = {0.0, 0.125, 0.25, 0.5, 1.0};

// TODO split this fn into 2, to allow manual inversion / reflection etc
static float S_LUT_interp( float  samp
                         , float  origin
                         , float  dest
                         , int    LUTsize
                         , float* LUT
                         ){
    // find samp's point between origin and dest (0-1.0)
    sinterp = (samp-origin) / dest;
    interp = origin + sinterp * (dest - origin);

    // find appropriate LUT neighbours & make coefficients
    float lut_i = interp * LUTsize;         // eg: 3.3
    int   lut_i_a = (int)lut_i;             //     3
    int   lut_i_b = int_i_a + 1;            //     4
    float lut_i_c = lut_i - (float)lut_i_a; //     0.3

    // pull LUT refs and interpolate
    float a = LUT[lut_i_a];
    float b = LUT[lut_i_b];
    return a + lut_i_c * (b - a);
}

float S_relative( float     samp   // sample value (absolute)
                , S_shape_t type   // choose the type
                , float     origin // zero point for continuous fn
                , float     dest   // end point for cont fn
                ){
    switch type{

    case S_linear:
        return samp;

    case S_square:
        // TODO could add a 'delay' style which always maps to origin
        return dest; // just map to the destination immediately

    case S_log:
        return S_LUT_interp( samp
                           , origin
                           , dest
                           , LUT_log_sz
                           , LUT_log
                           );

    case S_sine:
        // might need an extra step to handle inversion / negatives?
        return S_LUT_interp( samp
                           , origin
                           , dest
                           , LUT_sine_sz
                           , LUT_sine
                           );

    default: return samp; // default to linear
    }
}

float S_absolute( float samp
                , int   divs // 12 // must be >0
                , float fold // 1.0
                //, float zero // assume 0.0 for now
                ){
    // TODO optimize more elements into k-rate
    // k-rate
    float div_sz = fold / (float)divs; // abs size of a div
    // a-rate
    int bracket  = (int)(samp / fold); // confirm this rounds to -ve
    float shelf  = samp % fold; // 0-fold range
    float shelf1 = shelf / fold; // 0-1 range within fold
    int div      = (int)(0.5 + (shelf1 * (int)divs)); // select the div
    return ( (float)div * div_sz
             + ((float)bracket * fold) // add #folds back
           );
}
