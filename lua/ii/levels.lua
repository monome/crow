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
    , get  = false
    , docs = 'reset positions'
    }
  , { name = 'position'
    , cmd  = 2
    , get  = true
    , docs = 'set positions'
    , args = { { 'track', u8 }
             , { 'pos', u8 } 
           }
    }
  , { name = 'loop_start'
    , cmd  = 3
    , get  = true
    , docs = 'set loop start'
    , args = { { 'track', u8 }
             , { 'pos', u8 }
           }
    }
  , { name = 'loop_length'
    , cmd  = 4
    , get  = true
    , docs = 'set loop loop length'
    , args = { { 'track', u8 }
             , { 'pos', u8 }
           }
    }
  , { name = 'loop_direction'
    , cmd  = 5
    , get  = true 
    , docs = 'set loop direction'
    , args = { { 'track', u8 }
             , { 'direction', u8 }
           } 
    }
  , { name = 'cv'
    , cmd  = 6 
    , get  = true
    , docs = 'set cv'
    , args = { { 'track', u8 }
             , { 'volts', u16 }
           }
    }       
  }
}
end