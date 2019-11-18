do return
{ module_name  = 'ansible levels'
, manufacturer = 'monome'
, i2c_address  = 0x2c
, lua_name     = 'levels'
, commands     =
  { { name = 'preset'
    , cmd  = 0
    , get  = true
    , docs = 'set preset'
    , args = { 'number', u8 }
    }
  , { name = 'reset'
    , cmd  = 1
    , docs = 'reset positions. 0 = on next tick. 1 = now'
    , args = { 'now', u8 }
    }
  , { name = 'position'
    , cmd  = 2
    , get  = true
    , docs = 'set positions'
    , args = { 'pos', u8 }
    }
  , { name = 'loop_start'
    , cmd  = 3
    , get  = true
    , docs = 'set loop start'
    , args = { 'pos', u8 }
    }
  , { name = 'loop_length'
    , cmd  = 4
    , get  = true
    , docs = 'set loop loop length'
    , args = { 'pos', u8 }
    }
  , { name = 'loop_direction'
    , cmd  = 5
    , get  = true
    , docs = 'set loop direction'
    , args = { 'direction', u8 }
    }
  }
, getters =
  { { name = 'cv'
    , cmd  = 6 + get_offset
    , docs = 'get voltage on the provided track'
    , args = { 'track', u8 }
    , retval = { 'volts', s16V }
    }
  }
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
, pickle = 'if( data[0] == (128+6) ){ data[1] -= 1; }  // zero-index the cv request'
}
end
