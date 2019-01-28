do return
{ module_name  = 'W/'
, manufacturer = 'mannequins'
, i2c_address  = 0x71
, lua_name     = 'wslash'
, commands     =
  { { name = 'record'
    , cmd  = 1
    , get  = true
    , docs = 'Sets whether recording is active'
    , args = { 'active', s8 }
    }
  , { name = 'play'
    , cmd  = 2
    , get  = true
    , docs = 'Set the direction of playback regardless of mode (-1,0,1)'
    , args = { 'direction', s8 }
    }
  , { name = 'loop'
    , cmd  = 3
    , get  = true
    , docs = 'Set the status of looing'
    , args = { 'state', s8 }
    }
  , { name = 'cue'
    , cmd  = 4
    , get  = true
    , docs = 'Jump to a cuepoint, relative to current playhead'
    , args = { 'destination', s8 }
    }
  }
}
end
