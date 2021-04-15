do return
{ module_name  = 'crow'
, manufacturer = 'monome & whimsical raps'
, i2c_address  = {0x01, 0x02, 0x03, 0x04}
, lua_name     = 'crow'
, commands     =
  { { name = 'volts'
    , cmd  = 1
    , args = { {'channel', s8, }
             , {'level', s16V }
             }
    }
  , { name = 'slew'
    , cmd  = 2
    , args = { {'channel', s8, }
             , {'time', s16ms }
             }
    }
  , { name = 'call1'
    , cmd  = 4
    , args = { 'arg', s16 }
    }
  , { name = 'call2'
    , cmd  = 5
    , args = { { 'arg1', s16 }
             , { 'arg2', s16 }
             }
    }
  , { name = 'call3'
    , cmd  = 6
    , args = { { 'arg1', s16 }
             , { 'arg2', s16 }
             , { 'arg3', s16 }
             }
    }
  , { name = 'call4'
    , cmd  = 7
    , args = { { 'arg1', s16 }
             , { 'arg2', s16 }
             , { 'arg3', s16 }
             , { 'arg4', s16 }
             }
    }
  , { name = 'reset'
    , cmd  = 8
    }
  , { name = 'pulse'
    , cmd  = 9
    , args = { { 'chan', s8 }
             , { 'time', s16ms }
             , { 'level', s16V }
             , { 'polarity', s8 }
             }
    }
  , { name = 'ar'
    , cmd  = 10
    , args = { { 'chan', s8 }
             , { 'attack', s16ms }
             , { 'release', s16ms }
             , { 'level', s16V }
             }
    }
  , { name = 'lfo'
    , cmd  = 11
    , args = { { 'chan', s8 }
             , { 'freq', s16V } -- 0 == 1Hz
             , { 'level', s16V }
             , { 'skew', s16V }
             }
    }
  }
, getters =
  { { name   = 'input'
    , cmd    = 3 + get_offset
    , args   = { 'channel', s8 }
    , retval = { 'value', s16V }
    }
  , { name   = 'output'
    , cmd    = 4 + get_offset
    , args   = { 'channel', s8 }
    , retval = { 'value', s16V }
    }
  , { name   = 'query0'
    , cmd    = 5 + get_offset
    , retval = { 'value', s16 }
    }
  , { name   = 'query1'
    , cmd    = 6 + get_offset
    , args   = { 'arg1', s16 }
    , retval = { 'value', s16 }
    }
  , { name   = 'query2'
    , cmd    = 7 + get_offset
    , args   = { { 'arg1', s16 }
               , { 'arg2', s16 }
               }
    , retval = { 'retval', s16 }
    }
  , { name   = 'query3'
    , cmd    = 8 + get_offset
    , args   = { { 'arg1', s16 }
               , { 'arg2', s16 }
               , { 'arg3', s16 }
               }
    , retval = { 'value', s16 }
    }
  }
}
end
