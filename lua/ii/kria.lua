do return
{ module_name  = 'ansible kria'
, manufacturer = 'monome'
, i2c_address  = 0x28
, lua_name     = 'kria'
, commands     =
  { { name = 'preset'
    , cmd  = 0
    , get  = true
    , docs = 'set preset'
    , args = { 'number', u8 }
    }
  , { name = 'pattern'
    , cmd  = 1
    , get  = true
    , docs = 'set pattern'
    , args = { 'number', u8 }
    }
  , { name = 'scale'
    , cmd  = 2
    , get  = true
    , docs = 'set scale'
    , args = { 'number', u8 }
    }
  , { name = 'period'
    , cmd  = 3
    , get  = true
    , docs = 'set internal clock period'
    , args = { 'time', u16 }
    }
  , { name = 'position'
    , cmd  = 4
    , get  = true
    , docs = 'set position'
    , args = { { 'track', u8 }
             , { 'param', u8 }
             , { 'pos', u8 }
           }
    }
  , { name = 'loop_start'
    , cmd  = 5
    , get  = true
    , docs = 'set loop start'
    , args = { { 'track', u8 }
             , { 'param', u8 }
             , { 'pos', u8 }
           }
    }
  , { name = 'loop_length'
    , cmd  = 6
    , get  = true
    , docs = 'set loop length'
    , args = { { 'track', u8 }
             , { 'param', u8 }
             , { 'pos', u8 }
           }
    }
  , { name = 'reset'
    , cmd  = 7
    , get  = true
    , docs = 'reset position to start'
    , args = { { 'track', u8 }
             , { 'param', u8 }
           }
    }
  -- CMD 8, cv, only has getter??
  , { name = 'mute'
    , cmd  = 9
    , get  = true
    , docs = 'set mute state'
    , args = { { 'track', u8 }
             , { 'state', u8 }
           }
    }
  , { name = 'toggle_mute'
    , cmd  = 10
    , get  = false
    , docs = 'toggle mute state'
    , args = { 'track', u8 }
    }
  , { name = 'clock'
    , cmd  = 11
    , get  = false
    , docs = 'clock'
    , args = { 'track', u8 }
    }
  , { name = 'page'
    , cmd = 12
    , docs = 'get/set active parameter page'
    , args = { 'page', u8 }
    }
  , { name = 'cue'
    , cmd  = 13
    , args = { 'pattern', u8 }
    , docs = 'get/set next pattern to play'
    }
  , { name = 'direction'
    , cmd = 14
    , args = { { 'track', u8 }
             , { 'direction', u8 }
	     }
    , docs = 'get/set track step direction'
    }
  }
, getters =
  { { name = 'cv'
    , cmd  = 8 + get_offset
    , args = { 'track', u8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'duration'
    , cmd = 15 + get_offset
    , args = { { 'track', u8 } }
    , retval = { 'ms', u16 }
    }
  }
}
end
