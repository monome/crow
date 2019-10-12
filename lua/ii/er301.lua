do return
{ module_name  = 'ER-301'
, manufacturer = 'Orthogonal Devices'
, i2c_address  = 0x31
, lua_name     = 'er301' -- NB: must match the file name. er301.lua -> 'er301'
, commands     =
  { { name = 'tr'
    , cmd  = 0
    , docs = 'Set TR *port* to *value* (0/1)'
    , args = { { 'port', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'tr_tog'
    , cmd  = 1
    , docs = 'Toggle TR *port*'
    , args = { 'port', u8 }
    }
  , { name = 'tr_time'
    , cmd  = 2
    , docs = 'Time for TR.PULSE *port* in *value* milliseconds'
    , args = { { 'port', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'tr_pulse'
    , cmd  = 5
    , docs = 'Pulse TR *port* using TO.TR.TIME/S/M as an interval'
    , args = { 'port', u8 }
    }
  , { name = 'tr_pol'
    , cmd  = 6
    , docs = 'Polarity for TO.TR.PULSE *port* set to *value* (0/1)'
    , args = { { 'port', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'cv'
    , cmd  = 10
    , docs = 'Set *port* CV to *value* (bipolar), following SLEW time'
    , args = { { 'port', u8 }
             , { 'value', s16V }
             }
    }
  , { name = 'cv_set'
    , cmd  = 11
    , docs = 'Set CV *port* to *value* (bipolar), ignoring SLEW time'
    , args = { { 'port', u8 }
             , { 'value', s16V }
             }
    }
  , { name = 'cv_slew'
    , cmd  = 12
    , docs = 'CV *port* slew time in *value* milliseconds'
    , args = { { 'port', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'cv_off'
    , cmd  = 15
    , docs = 'CV *port* offset, *value* added at final stage'
    , args = { { 'port', u8 }
             , { 'value', s16V }
             }
    }
  }
}
end