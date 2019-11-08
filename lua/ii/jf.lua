do return
{ module_name  = 'just friends'
, manufacturer = 'mannequins'
, i2c_address  = 0x70
, lua_name     = 'jf' -- NB: must match the file name. jf.lua -> 'jf'
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
    , docs = 'Activate RUN if *mode* is non-zero'
    , args = { 'mode', s8 }
    }
  , { name = 'run'
    , cmd  = 3
    , docs = 'Set RUN value to *volts*. Requires run_mode(1)'
    , args = { 'volts', s16V }
    }
  , { name = 'transpose'
    , cmd  = 4
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
    , docs = 'If *mode* is non-zero, enter Synthesis / Geode'
    , args = { 'mode', s8 }
    }
  , { name = 'tick'
    , cmd  = 7
    , docs = 'Set geode tempo to *bpm*, or accept *clock* for geode'
    , args = { 'clock-or-bpm', s8 }
    }
  , { name = 'play_voice'
    , cmd  = 8
    , docs = 'Synthesis: Set *channel* to *pitch* at *level*\n\r' ..
        'Geode: At *channel* make *repeats* envs with *divs* time'
    , args = { { 'channel', s8 }
             , { 'pitch/divs', s16V }
             , { 'level/repeats', s16V }
             }
    }
  , { name = 'play_note'
    , cmd  = 9
    , docs = 'Synthesis: Assign a note with *pitch* at *level*\n\r' ..
        'Geode: Create *repeats* envelopes with *divs* timing'
    , args = { { 'pitch/divs', s16V }
             , { 'level/repeats', s16V }
             }
    }
  , { name = 'god_mode'
    , cmd  = 10
    , docs = 'If *state* is non-zero, shift pitch base to A=432Hz'
    , args = { 'state', s8 }
    }
  , { name = 'retune'
    , cmd  = 11
    , docs = 'Redefine INTONE of *channel* to (*num*/*denom*)'
    , args = { { 'channel', s8 }
             , { 'numerator', s16 }
             , { 'denominator', s16 }
             }
    }
  , { name = 'quantize'
    , cmd  = 12
    , docs = 'Quantize commands to every (bar/*divisions*)'
    , args = { 'divisions', s8 }
    }
  }
}
end
