do return
{ module_name  = 'ER-301'
, manufacturer = 'Orthogonal Devices'
, i2c_address  = {0x31, 0x32, 0x33}
, lua_name     = 'er301' -- NB: must match the file name. er301.lua -> 'er301'
, commands     =
  { { name = 'tr'
    , cmd  = 0x0
    , docs = 'Set TR *port* to *state* (0/1)'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'tr_tog'
    , cmd  = 0x1
    , docs = 'Toggle TR *port*'
    , args = { 'port', u8 }
    }
  , { name = 'tr_pulse'
    , cmd  = 0x5
    , docs = 'Pulse TR *port* using TO.TR.TIME/S/M as an interval'
    , args = { 'port', u8 }
    }
  , { name = 'tr_time'
    , cmd  = 0x2
    , docs = 'Time for TR.PULSE *port* in *ms*'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'tr_pol'
    , cmd  = 0x6
    , docs = 'Polarity for TO.TR.PULSE *port* set to *rising* (0/1)'
    , args = { { 'port', u8 }
             , { 'rising', s16 }
             }
    }
  , { name = 'cv'
    , cmd  = 0x10
    , docs = 'Set *port* CV to *volts* (bipolar), following SLEW time'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'cv_slew'
    , cmd  = 0x12
    , docs = 'CV *port* slew time in *ms*'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'cv_set'
    , cmd  = 0x11
    , docs = 'Set CV *port* to *volts* (bipolar), ignoring SLEW time'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'cv_off'
    , cmd  = 0x15
    , docs = 'CV *port* offset, *volts* added at final stage'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  }
, pickle = -- zero-index the port & send to multiple devices for port >= 100
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
[[

uint8_t port = data[1] - 1;  // zero-index the port
data[1]   = port % 100;      // wrap command for subsequent devices
*address += port / 100;      // increment address for subsequent devices

]]
}
end
