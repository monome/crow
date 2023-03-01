do return
{ module_name  = 'Disting EX'
, manufacturer = 'Expert Sleepers'
, i2c_address  = { 0x41, 0x42, 0x43, 0x44 }
, lua_name     = 'disting'
, commands     =
  { { name = 'load_preset'
    , cmd = 0x40
    , docs = 'load a preset'
    , args = { 'number', u16 }
    }
  , { name = 'save_preset'
    , cmd = 0x41
    , docs = 'save as a preset'
    , args = { 'value', u16 }
    }
  , { name = 'reset_preset'
    , cmd = 0x42
    }
  , { name = 'algorithm'
    , cmd = 0x44
    , docs = 'load single mode algorithm'
    , args = { 'algorithm', u8 }
    }
  , { name = 'controller'
    , cmd = 0x11
    , docs = 'sets parameters via mappings'
    , args = { { 'controller', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'parameter'
    , cmd = 0x46
    , args = { { 'parameter', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'scale_parameter'
    , cmd = 0x47
    , docs = 'The 0-16384 value range scales to the actual param. value range'
    , args = { { 'parameter', u8 }
             , { 'value', u16 }
             }
    }
  -- these are algorithm-specific
  , { name = 'wav_record'
    , cmd = 0x4B
    , docs = '0 for stop; 1 for start'
    , args = { 'action', u8 }
    }
  , { name = 'wav_playback'
    , cmd = 0x4C
    , args = { 'action', u8 }
    }
  , { name = 'al_pitch'
    , cmd = 0x4D
    , args = { 'pitch', u16 }
    , docs = 'Set Augustus Loop pitch'
    }
  , { name = 'al_clock'
    , cmd = 0x4E
    , docs = 'Send Augustus Loop clock'
    }
  --Looper
  , {
      name = 'lp_clear'
    , cmd = 0x58
    , docs = 'Clear target loop in Looper'
    }
  --MIDI / select bus
  , { name = 'midi'
    , cmd = 0x4F
    , args = { { 'status', u8 }
             , { 'b0', u8 }
             , { 'b1', u8 }
             }
    }
  , { name = 'select_bus'
    , cmd = 0x50
    , args = { { 'status', u8 }
             , { 'b0', u8 }
             , { 'b1', u8 }
             }
    }
  -- voice control
  , { name = 'voice_pitch'
    , cmd = 0x51
    , docs = 'Set voice pitch for the specified voice'
    , args = { { 'voice', u8 }
             , { 'pitch', s16V }
             }
    }
  , { name = 'voice_on'
    , cmd = 0x52
    , docs = 'note on for the specified voice'
    , args = { { 'voice', u8 }
             , { 'velocity', s16 }
             }
    }
  , { name = 'voice_off'
    , cmd = 0x53
    , docs = 'note off for the specified voice'
    , args = { 'voice', u8 }
    }
  , { name = 'note_pitch'
    , cmd = 0x54
    , docs = 'set voice pitch for note id'
    , args = { { 'note_id', u8 }
             , { 'pitch', s16V }
             }
    }
  , { name = 'note_velocity'
    , cmd = 0x55
    , docs = 'set voice velocity for note id'
    , args = { { 'note_id', u8 }
             , { 'velocity', s16V }
             }
    }
  , { name = 'note_off'
    , cmd = 0x56
    , docs = 'note off for note id'
    , args = { 'note_id', u8 }
    }
  , { name = 'all_notes_off'
    , cmd = 0x57
    }
  -- dual mode algorithms
  , { name = 'dual_parameter'
    , cmd = 0x5D
    , docs = 'set side/parameter to actual value'
    , args = { { 'side', u8 }
             , { 'param', u8 }
             , { 'value', s16 }
             }
    }
  , { name = 'dual_scale_parameter'
    , cmd = 0x5E
    , docs = 'The 0-16384 value range scales to the actual param. value range'
    , args = { { 'side', u8 }
             , { 'param', u8 }
             , { 'value', u16 }
             }
    }
  , { name = 'dual_algorithm'
    , cmd = 0x60
    , docs = 'load dual mode algorithm'
    , args = { { 'side', u8 }
             , { 'algorithm', u8 }
             }
    }
  , { name = 'dual_algorithms'
    , cmd = 0x62
    , docs = 'load dual mode algorithms'
    , args = { { 'algorithm_left', u8 }
             , { 'algorithm_right', u8 }
             }
    }
  , { name = 'load_dual_preset'
      , cmd = 0x63
      , docs = 'load dual mode preset'
      , args = { { 'side', u8 }
               , { 'preset', u8 }
               }
    }
  , { name = 'save_dual_preset'
      , cmd = 0x64
      , docs = 'save dual mode preset'
      , args = { { 'side', u8 }
               , { 'preset', u8 }
               }
    }
  , { name = 'take_over_z'
      , cmd = 0x65
      , docs = 'take over/release z. 0-127 = take over, else release'
      , args = { { 'side', u8 }
               , { 'value', u8 }
               }
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
    , args = { 'parameter', u8 }
    , retval = { 'value', s16 }
    }
  , { name = 'parameter_min'
    , cmd = 0x49
    , args = { 'parameter', u8 }
    , retval = { 'value', s16 }
    }
  , { name = 'parameter_max'
    , cmd = 0x4A
    , args = { 'parameter', u8 }
    , retval = { 'value', s16 }
    }
  , { name = 'lp_state'
    , cmd = 0x59
    , args = { 'loop', u8 }
    , retval = { 'state', u8 }
    , docs = 'Get target loop state in Looper, 0-based loop index'
    }
  , { name = 'dual_parameter'
    , cmd = 0x5A
    , args = { { 'side', u8 }
             , { 'parameter', u8 }
             }
    , retval = { 'value', u8 }
    }
  , { name = 'dual_parameter_min'
    , cmd = 0x5B
    , args = { { 'side', u8 }
             , { 'parameter', u8 }
             }
    , retval = { 'value', u8 }
    }
  , { name = 'dual_parameter_max'
    , cmd = 0x5C
    , args = { { 'side', u8 }
             , { 'parameter', u8 }
             }
    , retval = { 'value', u8 }
  , { name = 'dual_algorithm'
    , cmd = 0x5F
    , args = { 'side', u8 }
    , retval = { 'value', u8 }
    }   }
  }
, pickle = -- send side/parameter as one byte
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
[[

if (data[0] >= 0x5A &&
    data[0] <= 0x5E) {
  data[1] = ((data[1] & 0xF) << 4) | (data[2] & 0xF);

  if (data[0] >= 0x5C) {
    memmove(&data[2], &data[3], 2);
  }
  *byte_count = *byte_count - 1;
}

]]

}
end
