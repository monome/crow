do return
{ module_name  = 'TXi'
, manufacturer = 'bpc'
, i2c_address  = 0x68
, lua_name     = 'txi'
, commands     =
  { { name = 'nop'
    , cmd  = 2
    , docs = 'place holder'
    , args = { { 'channel', s8 }
             , { 'state', s8 }
             }
    }
  }
, getters =
  { { name = 'value'
    , cmd  = 1
    , args = { 'channel', s8 }
    , retval = { 'value', u16 }
    }
  }
}
end
