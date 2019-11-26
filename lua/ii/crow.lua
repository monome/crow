do return
{ module_name  = 'crow'
, manufacturer = 'monome & whimsical raps'
, i2c_address  = 0x01
, lua_name     = 'crow'
, commands     =
  { { name = 'output'
    , cmd  = 1
    , args = { {'channel', s8, }
             , {'level', s16V }
             }
    }
  , { name = 'slew'
    , cmd  = 2
    , args = { {'channel', s8, }
             , {'time', s16 }
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
