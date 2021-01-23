do return
{ module_name  = 'W/ delay'
, manufacturer = 'mannequins'
, i2c_address  = {0x78, 0x79}
, lua_name     = 'wdel' -- NB: must match the file name. jf.lua -> 'jf'
, commands     =
  { { name = 'feedback'
    , cmd  = 2
    , docs = 'amount of feedback from read head to write head'
    , get  = true
    , args = { 'level', s16V }
    }
  , { name = 'mix'
    , cmd  = 3
    , docs = 'fade from dry to delayed signal'
    , get  = true
    , args = { 'fade', s16V }
    }
  , { name = 'filter'
    , cmd  = 4
    , docs = 'centre frequency of filter in feedback loop'
    , get  = true
    , args = { 'cutoff', s16V }
    }
  , { name = 'freeze'
    , cmd  = 5
    , docs = 'deactivate record head to freeze the current buffer'
    , get  = true
    , args = { 'is_active', s8 }
    }
  , { name = 'time'
    , cmd  = 6
    , docs = 'set delay buffer length in seconds, when rate == 1'
    , get  = true
    , args = { 'seconds', s16V }
    }
  , { name = 'length'
    , cmd  = 7
    , docs = 'set buffer loop size as a fraction of buffer time'
    , args = { { 'count', u8 }
             , { 'divisions', u8 }
             }
    }
  , { name = 'position'
    , cmd  = 8
    , docs = 'set loop location as a fraction of buffer time'
    , args = { { 'count', u8 }
             , { 'divisions', u8 }
             }
    }
  , { name = 'cut'
    , cmd  = 9
    , docs = 'jump to loop location as a fraction of buffer time'
    , args = { { 'count', u8 }
             , { 'divisions', u8 }
             }
    }
--  , { name = 'freq_range'
--    , cmd  = 10
--    , docs = 'TBD'
--    , get  = true
--    , args = { 'range', s8 }
--    }
  , { name = 'rate'
    , cmd  = 11
    , get  = true
    , docs = 'direct multiplier of tape speed'
    , args = { 'multiplier', s16V }
    }
  , { name = 'freq'
    , cmd  = 12
    , docs = 'manipulate tape speed with musical values'
    , get  = true
    , args = { 'volts', s16V }
    }
  , { name = 'clock'
    , cmd  = 13
    , docs = 'receive clock pulse for synchronization'
    }
  , { name = 'clock_ratio'
    , cmd  = 14
    , docs = 'set clock pulses per buffer time, with clock mul/div'
    , args = { { 'mul', s8 }
             , { 'div', s8 }
             }
    }
  , { name = 'pluck'
    , cmd  = 15
    , docs = 'pluck the delay line with noise at volume'
    , args = { 'volume', s16V }
    }
  , { name = 'mod_rate'
    , cmd  = 16
    , get  = true
    , docs = 'set the multiplier for the modulation rate'
    , args = { 'rate', s16V }
    }
  , { name = 'mod_amount'
    , cmd  = 17
    , get  = true
    , docs = 'set the amount of delay line modulation to be applied'
    , args = { 'amount', s16V }
    }
  }
}
end
