do return
{ module_name  = 'just friends'
, manufacturer = 'mannequins'
, i2c_address  = 0x70
, lua_name     = 'jf'
, commands     =
  { { name = 'trigger'
    , cmd  = 1
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
    , args = { 'volts', s16 }
    }
  , { name = 'transpose'
    , cmd  = 4
    , get  = true
    , docs = 'Add *pitch* to the current TIME setting'
    , args = { 'pitch', s16 }
    }
  , { name = 'vtrigger'
    , cmd  = 5
    , docs = 'Set TRIGGER *channel* to *level*'
    , args = { { 'channel', s8 }
             , { 'level', s16 }
             }
    }
  , { name = 'retune'
    , cmd  = 6
    , docs = 'Redefine INTONE of *channel* to (*num*/*denom*)'
    , args = { { 'channel', s8 }
             , { 'numerator', s16 }
             , { 'denominator', s16 }
             }
    }
  , { name = 'mode'
    , cmd  = 7
    , get  = true
    , docs = 'If *mode* is non-zero, enter Synthesis / Geode'
    , args = { 'mode', s8 }
    }
  , { name = 'play_voice'
    , cmd  = 8
    , docs = 'Synthesis: Set *channel* to *pitch* at *level*\n\r' ..
        'Geode: At *channel* make *repeats* envs with *divs* time'
    , args = { { 'channel', s8 }
             , { 'pitch/divs', s16 }
             , { 'level/repeats', s16 }
             }
    }
  , { name = 'play_note'
    , cmd  = 9
    , docs = 'Synthesis: Assign a note with *pitch* at *level*\n\r' ..
        'Geode: Create *repeats* envelopes with *divs* timing'
    , args = { { 'pitch/divs', s16 }
             , { 'level/repeats', s16 }
             }
    }
  , { name = 'god_mode'
    , cmd  = 10
    , get  = true
    , docs = 'If *state* is non-zero, shift pitch base to A=432Hz'
    , args = { 'state', s8 }
    }
  , { name = 'tick'
    , cmd  = 11
    , get  = true
    , docs = 'Set geode tempo to *bpm*, or accept *clock* for geode'
    , args = { 'clock-or-bpm', s8 }
    }
  , { name = 'quantize'
    , cmd  = 12
    , get  = true
    , docs = 'Quantize commands to every (bar/*divisions*)'
    , args = { 'divisions', s8 }
    }
  }
, getters =
  { { name = 'retune'
    , cmd  = 6 + get_offset
    , args = { 'channel', s8 }
    , retval = { 'numerator', s8 }
    }
  }
}
end
