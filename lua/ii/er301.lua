do return
{ module_name  = 'ER-301'
, manufacturer = 'Orthogonal Devices'
, i2c_address  = 0x31
, lua_name     = 'er301' -- NB: must match the file name. er301.lua -> 'er301'
, commands     =
  { { name = 'tr'
    , cmd  = 1
    , get  = true
    , docs = 'Set TR *port* to *value* (0/1)'
    , args = { { 'port', u8 }
             , { 'value', u8 }
             }
    }
  , { name = 'tr_tog'
    , cmd  = 2
    , docs = 'Toggle TR *port*'
    , args = { 'port', u8 }
    }
  , { name = 'tr_pulse'
    , cmd  = 3
    , docs = 'Pulse TR *port* using TO.TR.TIME/S/M as an interval'
    , args = { 'port', u8 }
    }
  , { name = 'tr_time'
    , cmd  = 4
    , get  = true
    , docs = 'Time for TR.PULSE *port* in *value* milliseconds'
    , args = { { 'port', u8 }
             , { 'value', u16 }
             }
    }
  , { name = 'tr_pol'
    , cmd  = 5
    , get  = true
    , docs = 'Polarity for TO.TR.PULSE *port* set to *value* (0/1)'
    , args = { { 'port', u8 }
             , { 'value', u8 }
             }
    }
  , { name = 'cv'
    , cmd  = 6
    , get  = true
    , docs = 'Set *port* CV to *value* (bipolar), following SLEW time'
    , args = { { 'port', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'cv_slew'
    , cmd  = 7
    , get  = true
    , docs = 'CV *port* slew time in *value* milliseconds'
    , args = { { 'port', u8 }
             , { 'value', u16 }
             }
    }
  , { name = 'cv_set'
    , cmd  = 8
    , get  = true
    , docs = 'Set CV *port* to *value* (bipolar), ignoring SLEW time'
    , args = { { 'port', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'cv_off'
    , cmd  = 9
    , get  = true
    , docs = 'CV *port* offset, *value* added at final stage'
    , args = { { 'port', u8 }
             , { 'value', u16 }
             }
}
end