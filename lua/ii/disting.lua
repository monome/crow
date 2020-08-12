do return
{ module_name  = 'Disting EX'
, manufacturer = 'Expert Sleepers'
, i2c_address  = { 0x41, 0x42, 0x43, 0x44 }
, lua_name     = 'disting'
, commands     =
  { { name = 'load_preset'
    , cmd = 0x40
    , args = { 'value', u16 }
    }
  , { name = 'save_preset'
    , cmd = 0x41
    , args = { 'value', u16 }
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
    , args = { { 'controller', u8 }
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
  -- TODO: MIDI / select bus
  , { name = 'midi'
    , cmd = 0x4F
    , args = { { 'status', u8 }
             , { 'b1', u8 }
	     , { 'b2', u8 }
	     }
    }
  -- voice control
  , { name = 'voice_pitch'
    , cmd = 0x51
    , args = { { 'voice', u8 }
             , { 'pitch', s16 }
	     }
    }
  , { name = 'note_on'
    , cmd = 0x52
    , args = { { 'voice', u8 }
             , { 'velocity', s16 }
             }
    }
  , { name = 'note_off'
    , cmd = 0x53
    , args = { 'voice', u8 }
    }
  }
, getters =
  { { name = 'preset'
    , cmd = 0x43
    , retval = { 'preset', u16 }
    }
  , { name = 'algorithm'
    , cmd = 0x45
    , retval = { 'algorithm', u8 }
    }
  , { name = 'parameter'
    , cmd = 0x48
    , retval = { 'value', u16 }
    }
  , { name = 'parameter_min'
    , cmd = 0x49
    , args = { 'parameter', u16 }
    , retval = { 'value', u16 }
    }
  , { name = 'parameter_max'
    , cmd = 0x4A
    , args = { 'parameter', u16 }
    , retval = { 'value', u16 }
    }
  }
}
end
