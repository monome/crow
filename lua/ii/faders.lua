do return
{ module_name  = '16n Faderbank'
, manufacturer = '_'
, i2c_address  = {0x34, 0x35, 0x36, 0x37}
, lua_name     = 'faders'
, getters =
  { { name = 1
    , cmd  = 0
    , retval = { 'volts', s16V }
    }
  , { name = 2
    , cmd  = 1
    , retval = { 'volts', s16V }
    }
  , { name = 3
    , cmd  = 2
    , retval = { 'volts', s16V }
    }
  , { name = 4
    , cmd  = 3
    , retval = { 'volts', s16V }
    }
  , { name = 5
    , cmd  = 4
    , retval = { 'volts', s16V }
    }
  , { name = 6
    , cmd  = 5
    , retval = { 'volts', s16V }
    }
  , { name = 7
    , cmd  = 6
    , retval = { 'volts', s16V }
    }
  , { name = 8
    , cmd  = 7
    , retval = { 'volts', s16V }
    }
  , { name = 9
    , cmd  = 8
    , retval = { 'volts', s16V }
    }
  , { name = 10
    , cmd  = 9
    , retval = { 'volts', s16V }
    }
  , { name = 11
    , cmd  = 10
    , retval = { 'volts', s16V }
    }
  , { name = 12
    , cmd  = 11
    , retval = { 'volts', s16V }
    }
  , { name = 13
    , cmd  = 12
    , retval = { 'volts', s16V }
    }
  , { name = 14
    , cmd  = 13
    , retval = { 'volts', s16V }
    }
  , { name = 15
    , cmd  = 14
    , retval = { 'volts', s16V }
    }
  , { name = 16
    , cmd  = 15
    , retval = { 'volts', s16V }
    }
  }
}
end

--[[ example usage:

ii.faders.get(1)
ii.faders.event = function( e, data )
    -- e == fader
    -- data = volts
end

--]]
