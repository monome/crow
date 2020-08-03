do return
{ module_name  = 'ansible'
, manufacturer = 'monome'
, i2c_address  = { 0x20, 0x21, 0x22, 0x23 }
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
             , { 'volts', s16V }
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
             , { 'volts', s16V }
             }
    }
  , { name = 'cv_set'
    , cmd  = 9
    , get  = false
    , docs = 'set cv bypass slew'
    , args = { { 'channel', u8 }
             , { 'volts', s16V }
             }
    }
  -- no setter for CMD 10 input
  }
, getters =
  { { name = 'input'
    , cmd  = 10 + get_offset
    , args = { 'channel', u8 }
    , docs = 'get input state'
    , retval = { 'state', u8 }
    }
  }
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
, pickle =
[[

uint8_t ch = data[1] - 1; // zero-index the channel
data[1]   = ch % 4;       // wrap command per device
*address += ch / 4;       // increment address for subsequent devices

]]
}
end
