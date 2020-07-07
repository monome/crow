do return
{ module_name  = 'W/ synth'
, manufacturer = 'mannequins'
, i2c_address  = {0x76, 0x77}
, lua_name     = 'wsyn' -- NB: must match the file name. jf.lua -> 'jf'
, commands     =
  { { name = 'velocity'
    , cmd  = 2
    , docs = 'strike the vactrol of <voice> at <velocity> in volts'
    , args = { { 'voice', s8 }
             , { 'velocity', s16V }
             }
    }
  , { name = 'pitch'
    , cmd  = 3
    , docs = 'set <voice> to <pitch> in volts-per-octave'
    , args = { { 'voice', s8 }
             , { 'pitch', s16V }
             }
    }
  , { name = 'play_voice'
    , cmd  = 4
    , docs = 'set <voice> to <pitch> (v8) and strike the vactrol at <velocity> (V)'
    , args = { { 'voice', s8 }
             , { 'pitch', s16V }
             , { 'velocity', s16V }
             }
    }
  , { name = 'play_note'
    , cmd  = 5
    , docs = 'dynamically assign a voice, set to <pitch> (v8), strike with <velocity>'
    , args = { { 'pitch', s16V }
             , { 'level', s16V }
             }
    }
  , { name = 'ar_mode'
    , cmd  = 7
    , docs = 'in attack-release mode, all notes are "plucked" and no "release" is required'
    , get  = true
    , args = { 'is_ar', s8 }
    }
  , { name = 'curve'
    , cmd  = 8
    , docs = 'cross-fade waveforms: -5=square, 0=triangle, 5=sine'
    , get  = true
    , args = { 'curve', s16V }
    }
  , { name = 'ramp'
    , cmd  = 9
    , docs = 'waveform symmetry: -5=rampwave, 0=triangle, 5=sawtooth (NB: affects FM tone)'
    , get  = true
    , args = { 'ramp', s16V }
    }
  , { name = 'fm_index'
    , cmd  = 10
    , docs = 'amount of FM modulation. -5=negative, 0=minimum, 5=maximum'
    , get  = true
    , args = { 'index', s16V }
    }
  , { name = 'fm_env'
    , cmd  = 16
    , docs = 'amount of vactrol envelope applied to fm index, -5 to +5'
    , get  = true
    , args = { 'amount', s16V }
    }
  , { name = 'fm_ratio'
    , cmd  = 11
    , docs = 'ratio of the FM modulator to carrier as a ratio. floating point values up to 20.0 supported'
    , args = { { 'numerator', s16V }
             , { 'denomenator', s16V }
             }
    }
  , { name = 'lpg_time'
    , cmd  = 12
    , docs = 'vactrol time constant. -5=drones, 0=vtl5c3, 5=blits'
    , get  = true
    , args = { 'time', s16V }
    }
  , { name = 'lpg_symmetry'
    , cmd  = 13
    , docs = 'vactrol attack-release ratio. -5=fastest attack, 5=long swells'
    , get  = true
    , args = { 'symmetry', s16V }
    }
  , { name = 'patch'
    , cmd  = 14
    , docs = 'patch a hardware *jack* to a *param* destination'
    , get  = true
    , args = { { 'jack' , s8 }
             , { 'param', s8 }
             }
    }
  , { name = 'voices'
    , cmd  = 15
    , docs = 'set number of polyphonic voices to allocate. use 0 for unison mode'
    , get  = true
    , args = { 'count' , s8 }
    }
  }
, getters =
  { { name = 'fm_ratio'
    , cmd  = 11 + get_offset
    , retval = { 'ratio', s16V }
    }
  }
}
end
