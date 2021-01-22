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
  , { name = 'erase_strength'
    , cmd  = 6
    , get  = true
    , docs = 'Strength of erase head when recording. 0 is overdub, 1 is overwrite. Opposite of feedback'
    , args = { 'level', s16V }
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
  , { name = 'echo_mode'
    , cmd  = 9
    , get  = true
    , docs = 'Set to 1 to playback before erase. 0 (default) erases first'
    , args = { 'is_echo', s8 }
    }
  , { name = 'loop_start'
    , cmd  = 10
    , docs = 'Set the current time as the beginning of a loop'
    }
  , { name = 'loop_end'
    , cmd  = 11
    , docs = 'Set the current time as the loop end, and jump to start'
    }
  , { name = 'loop_active'
    , cmd  = 12
    , args = { 'state', s8 }
    , docs = 'Set the state of looping'
    }
  , { name = 'loop_scale'
    , cmd  = 13
    , get  = true
    , args = { 'scale', s8 }
    , docs = 'Mul(Positive) or Div(Negative) loop brace by arg. Zero resets to original window'
    }
  , { name = 'loop_next'
    , cmd  = 14
    , docs = 'Move loop brace forward/backward by length of loop. Zero jumps to loop start'
    , args = { 'direction', s8 }
    }
  , { name = 'timestamp'
    , cmd  = 15
    , get  = true
    , docs = 'Move playhead to an arbitrary location on tape'
    , args = { 'seconds', s32T }
    }
  , { name = 'seek'
    , cmd  = 16
    , docs = 'Move playhead relative to current position'
    , args = { 'seconds', s32T }
    }
  , { name = 'WARNING_clear_tape'
    , cmd  = 18
    , docs = 'WARNING: Clears all audio on the tape! Unrecoverable!'
    }
  }
, getters =
  { { name   = 'speed'
    , cmd    = 4 + get_offset
    , docs   = 'Query the current tape speed'
    , retval = { 'rate', s16V }
    }
  , { name   = 'loop_start'
    , cmd    = 10 + get_offset
    , docs   = 'Query the timestamp of loop start point'
    , retval = { 'timestamp', s32T }
    }
  , { name   = 'loop_end'
    , cmd    = 11 + get_offset
    , docs   = 'Query the timestamp of loop end point'
    , retval = { 'timestamp', s32T }
    }
--- TT specific getters for reference only
--  , { name   = 'timestampS'
--    , cmd    = 17 + get_offset
--    , docs   = 'Query the current timestamp. returning only whole seconds'
--    , args = { 'seconds', s16 }
--    }
  }
}
end
