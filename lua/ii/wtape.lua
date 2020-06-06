do return
{ module_name  = 'W/ Tape'
, manufacturer = 'mannequins'
, i2c_address  = { 0x71, 0x72 }
, lua_name     = 'wtape'
, commands     =
  { { name = 'record'
    , cmd  = 1
    , get  = true
    , docs = 'Sets recording state to active'
    , args = { 'active', s8 }
    }
  , { name = 'play'
    , cmd  = 2
    , get  = true
    , docs = 'Set the playback state. -1 will flip playback direction'
    , args = { 'playback', s8 }
    }
  , { name = 'reverse'
    , cmd  = 3
    , docs = 'Reverse the direction of playback'
    }
  , { name = 'speed'
    , cmd  = 4
    , docs = 'Set speed as a rate, or ratio. Negative values are reverse'
    , args = { { 'speed_or_num', s16V }
             , { 'deno'        , s16V }
             }
    }
  , { name = 'freq'
    , cmd  = 5
    , get  = true
    , docs = 'Set speed as a frequency style value. Maintains reverse state'
    , args = { 'frequency', s16V }
    }
  , { name = 'pre_level'
    , cmd  = 6
    , get  = true
    , docs = 'Level of old recording left after a recording. aka feedback'
    , args = { 'gain', s16V }
    }
  , { name = 'monitor_level'
    , cmd  = 7
    , get  = true
    , docs = 'Level of input passed directly to output'
    , args = { 'gain', s16V }
    }
  , { name = 'rec_level'
    , cmd  = 8
    , get  = true
    , docs = 'Level of input material recorded to tape'
    , args = { 'gain', s16V }
    }
  , { name = 'head_order'
    , cmd  = 9
    , get  = true
    , docs = 'Set to 1 for playback before erasing. For destructive looping'
    , args = { 'previous', s8 }
    }
  , { name = 'loop_start'
    , cmd  = 10
    , docs = 'Set the current time as the beginning of a loop'
    }
  , { name = 'loop_end'
    , cmd  = 11
    , docs = 'Set the current time as the loop end, and jump to start'
    }
  , { name = 'loop_cancel'
    , cmd  = 12
    , docs = 'Cancel the current loop'
    }
  , { name = 'loop_reset'
    , cmd  = 13
    , docs = 'Jump playback to the start of the loop'
    }
  , { name = 'loop_next'
    , cmd  = 14
    , docs = 'Move loop brace forward/backward by length of loop'
    , args = { 'direction', s8 }
    }
  , { name = 'go'
    , cmd  = 15
    , docs = 'Move playhead to an arbitrary location on tape'
    , args = { 'seconds', s32T }
    }
  , { name = 'seek'
    , cmd  = 16
    , docs = 'Move playhead relative to current position'
    , args = { 'seconds', s32T }
    }
  }
}
end
