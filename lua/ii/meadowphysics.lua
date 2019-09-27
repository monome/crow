do return
{ module_name  = 'ansible meadowphysics'
, manufacturer = 'monome'
, i2c_address  = 0x2A
, lua_name     = 'meadowphysics'
, commands     =
  { { name = 'preset'
    , cmd  = 0
    , get  = true
    , docs = 'set preset'
    , args = { 'number', u8 }
    }
  , { name = 'reset'
    , cmd  = 1
    , get  = false
    , docs = 'reset position(s)'
    , args = { 'track', u8 }
    }
  , { name = 'stop'
    , cmd  = 2
    , get  = true
    , docs = 'stop track(s)'
    , args = { 'track', u8 }
    }
  , { name = 'scale'
    , cmd  = 3
    , get  = true
    , docs = 'set scale'
    , args = { 'number', u8 }
    }
  , { name = 'period'
    , cmd  = 4
    , get  = true
    , docs = 'set internal clock period'
    , args = { 'time', u16 }
    }
  }
, getters =
  { { name = 'cv'
    , cmd  = 5 + get_offset
    , args = { 'track', u8 }
    , retval = { 'volts', u16 }
    }
  }
}
end
