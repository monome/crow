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
