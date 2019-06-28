do return
{ module_name  = 'ansible'
, manufacturer = 'monome'
, i2c_address  = 0x20
, lua_name     = 'ansible'
, commands     =
  { { name = 'trigger'
    , cmd  = 1
    , get  = true
    , docs = 'set trigger state'
    , args = { { 'channel', u8 }
             , { 'state', u8 }
           }
    }
  , { name = 'trigger_toggle'
    , cmd  = 2
    , get  = false
    , docs = 'toggle trigger state'
    , args = { 'channel', u8 }
    }
  , { name = 'trigger_pulse'
    , cmd  = 3
    , get  = false
    , docs = 'pulse trigger'
    , args = { 'channel', u8 }
    }
  , { name = 'trigger_time'
    , cmd  = 4
    , get  = true
    , docs = 'set trigger time'
    , args = { { 'channel', u8 }
             , { 'time', u16 }
             }
    }
  , { name = 'trigger_polarity'
    , cmd  = 5
    , get  = true
    , docs = 'set trigger polarity'
    , args = { { 'channel', u8 }
             , { 'polarity', u8 }
             }
    }
  , { name = 'cv'
    , cmd  = 6
    , get  = true
    , docs = 'set cv'
    , args = { { 'channel', u8 }
             , { 'volts', u16 }
             }
    }
  , { name = 'cv_slew'
    , cmd  = 7
    , get  = true
    , docs = 'set cv slew'
    , args = { { 'channel', u8 }
             , { 'time', u16 }
             }
    }
  , { name = 'cv_offset'
    , cmd  = 8
    , get  = true
    , docs = 'set cv offset'
    , args = { { 'channel', u8 }
             , { 'volts', u16 }
             }
    }
  , { name = 'cv_set'
    , cmd  = 9
    , get  = false
    , docs = 'set cv bypass slew'
    , args = { { 'channel', u8 }
             , { 'volts', u16 }
             }
    }
  -- no setter for CMD 10 input
  }
, getters =
  { { name = 'cv'
    , cmd  = 8 + get_offset
    , args = { 'channel', u8 }
    , retval = { 'volts', u16 }
    }
  }
  , { name = 'input'
    , cmd  = 10 + get_offset
    , args = { '', u8 }
    --, docs = 'get input state'
    , retval = { 'state', u8 }
    }
}
end
