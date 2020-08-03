do return
{ module_name  = 'just friends'
, manufacturer = 'mannequins'
, i2c_address  = {0x70,0x75}
, lua_name     = 'jf' -- NB: must match the file name. jf.lua -> 'jf'
, commands     =
  { { name = 'trigger'
    , cmd  = 1
    , get  = true
    , docs = 'Set TRIGGER *channel* to *state*'
    , args = { { 'channel', s8 }
             , { 'state', s8 }
             }
    }
  , { name = 'run_mode'
    , cmd  = 2
    , get  = true
    , docs = 'Activate RUN if *mode* is non-zero'
    , args = { 'mode', s8 }
    }
  , { name = 'run'
    , cmd  = 3
    , get  = true
    , docs = 'Set RUN value to *volts*. Requires run_mode(1)'
    , args = { 'volts', s16V }
    }
  , { name = 'transpose'
    , cmd  = 4
    , get  = true
    , docs = 'Add *pitch* to the current TIME setting'
    , args = { 'pitch', s16V }
    }
  , { name = 'vtrigger'
    , cmd  = 5
    , docs = 'Set TRIGGER *channel* to *level*'
    , args = { { 'channel', s8 }
             , { 'level', s16V }
             }
    }
  , { name = 'mode'
    , cmd  = 6
    , get  = true
    , docs = 'If *mode* is non-zero, enter Synthesis / Geode'
    , args = { 'mode', s8 }
    }
  , { name = 'tick'
    , cmd  = 7
    , get  = true
    , docs = 'Set geode tempo to *bpm*, or accept *clock* for geode'
    , args = { 'clock_or_bpm', s8 }
    }
  , { name = 'play_voice'
    , cmd  = 8
    , docs = 'Synthesis: Set *channel* to *pitch* at *level*\n\r' ..
        'Geode: At *channel* make *repeats* envs with *divs* time'
    , args = { { 'channel', s8 }
             , { 'pitch_or_divs', s16V }
             , { 'level_or_repeats', s16V }
             }
    }
  , { name = 'play_note'
    , cmd  = 9
    , docs = 'Synthesis: Assign a note with *pitch* at *level*\n\r' ..
        'Geode: Create *repeats* envelopes with *divs* timing'
    , args = { { 'pitch_or_divs', s16V }
             , { 'level_or_repeats', s16V }
             }
    }
  , { name = 'god_mode'
    , cmd  = 10
    , get  = true
    , docs = 'If *state* is non-zero, shift pitch base to A=432Hz'
    , args = { 'state', s8 }
    }
  , { name = 'retune'
    , cmd  = 11
    , docs = 'Redefine INTONE of *channel* to (*num*/*denom*)'
    , args = { { 'channel', s8 }
             , { 'numerator', s8 }
             , { 'denominator', s8 }
             }
    }
  , { name = 'quantize'
    , cmd  = 12
    , get  = true
    , docs = 'Quantize commands to every (bar/*divisions*)'
    , args = { 'divisions', s8 }
    }
  , { name = 'pitch'
    , cmd  = 13
    , docs = 'Synthesis: Set *channel* to *pitch*'
    , args = { { 'channel', s8 }
             , { 'pitch', s16V }
             }
    }
  , { name = 'address'
    , cmd  = 14
    , docs = 'Set i2c address to *index* (default)1 or 2'
    , args = { 'index', s8 }
    }
  }
, getters =
  { { name = 'speed'
    , cmd  = 15 + get_offset
    , docs = 'Speed switch setting: 0 for shape and 1 for sound'
    , retval = { 'is_sound', s8 }
    }
  , { name = 'tsc'
    , cmd  = 16 + get_offset
    , docs = 'MODE switch setting: 0/1/2 for transient/sustain/cycle'
    , retval = { 'transient_sustain_cycle', s8 }
    }
  , { name = 'ramp'
    , cmd  = 17 + get_offset
    , docs = 'knob + cv for RAMP parameter'
    , retval = { 'volts', s16V }
    }
  , { name = 'curve'
    , cmd  = 18 + get_offset
    , docs = 'knob + cv for CURVE parameter'
    , retval = { 'volts', s16V }
    }
  , { name = 'fm'
    , cmd  = 19 + get_offset
    , docs = 'value of FM knob'
    , retval = { 'volts', s16V }
    }
  , { name = 'time'
    , cmd  = 20 + get_offset
    , docs = 'knob + cv for TIME parameter'
    , retval = { 'volts', s16V }
    }
  , { name = 'intone'
    , cmd  = 21 + get_offset
    , docs = 'knob + cv for INTONE parameter'
    , retval = { 'volts', s16V }
    }
  }
}
end
