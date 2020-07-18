do return
{ module_name  = 'Disting EX'
, manufacturer = 'Expert Sleepers'
, i2c_address  = { 0x41, 0x42, 0x43, 0x44 }
, lua_name     = 'disting'
, commands     =
  { { name = 'load_preset'
    , cmd = 0x40
    , args = { { 'msb', u8 }
             , { 'lsb', u8 }
             }
    }
  , { name = 'save_preset'
    , cmd = 0x41
    , args = { { 'msb', u8 }
             , { 'lsb', u8 }
             }
    }
  , { name = 'reset_preset'
    , cmd = 0x42
    }
  , { name = 'load_algorithm'
    , cmd = 0x44
    , args = { 'algorithm', u16 }
    }
  , { name = 'set_controller'
    , cmd = 0x11
    , args = { { 'controller', u16 }
             , { 'value', u16 }
             }
    }
  , { name = 'set_parameter'
    , cmd = 0x46
    , args = { { 'parameter', u16 }
             , { 'value', u16 }
             }
    }
  , { name = 'wav_record'
    , cmd = 0x4B
    -- TODO: args
    }
  , { name = 'wav_playback'
    , cmd = 0x4C
    -- TODO: args
    }
  , { name = 'al_pitch'
    , cmd = 0x4D
    , args = { 'pitch', u16 }
    }
  , { name = 'al_clock'
    , cmd = 0x4E
    }
  -- TODO: MIDI / select bus ++ voice control
  }
, getters =
  { { name = 'get_preset'
    , cmd = 0x43
    , retval = { 'preset', u16 }
    }
  , { name = 'get_algorithm'
    , cmd = 0x45
    , retval = { 'algorithm', u8 }
    }
  , { name = 'get_parameter'
    , cmd = 0x48
    , retval = { 'value', u16 }
    }
  , { name = 'get_parameter_min'
    , cmd = 0x49
    , args = { 'parameter', u16 }
    , retval = { 'value', u16 }
    }
  , { name = 'get_parameter_max'
    , cmd = 0x4A
    , args = { 'parameter', u16 }
    , retval = { 'value', u16 }
    }
  }
}
end
