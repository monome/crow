-- Please read the TT docs or https://github.com/bpcmusic/telex for full docs
-- nordseele januar 2020

do return
{ module_name  = 'TXO'
, manufacturer = 'BPC Music'
, i2c_address  = {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67}
, lua_name     = 'txo'
, commands     =

-- TXo Trigger Output (TR) Basic Commands

  { { name = 'tr'
    , cmd  = 0x0
    , docs = 'Set trigger output x to y (0-1)'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'tr_tog'
    , cmd  = 0x1
    , docs = 'Toggle TR port'
    , args = { 'port', u8 }
    }
  , { name = 'tr_pulse'
    , cmd  = 0x5
    , docs = 'Pulse trigger output x'
    , args = { 'port', u8 }
    }
  , { name = 'tr_time'
    , cmd  = 0x2
    , docs = 'Set the pulse time of trigger x to y ms'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'tr_pol'
    , cmd  = 0x6
    , docs = 'Set polarity of trigger output x to y (0-1)'
    , args = { { 'port', u8 }
             , { 'rising', s16 }
             }
    }

-- TXo Trigger Output (TR) Extended Commands

  , { name = 'tr_time_s'
    , cmd  = 0x3
    , docs = 'Time for TR.PULSE port in s'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'tr_time_m'
    , cmd  = 0x4
    , docs = 'Time for TR.PULSE port in m'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }

-- TXo Trigger Output (TR) Experimental Commands - Divider + Metronomes

  , { name = 'tr_pulse_div'
    , cmd  = 0x7
    , docs = 'Pulse divider for TR output # of pulses'
    , args = { { 'port', u8 }
             , { 'pulses', s16 }
             }
    }
  , { name = 'tr_m'
    , cmd  = 0x8
    , docs = 'Time for TR.M port in ms'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'tr_m_s'
    , cmd  = 0x9
    , docs = 'Time for TR.M port in s'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'tr_m_m'
    , cmd  = 0x0A
    , docs = 'Time for TR.M port in m'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }
  , { name = 'tr_m_bpm'
    , cmd  = 0x0B
    , docs = 'Time for TR.M port in bpm'
    , args = { { 'port', u8 }
             , { 'bpm', s16 }
             }
    }
  , { name = 'tr_m_act'
    , cmd  = 0x0C
    , docs = 'Activates the metronome for the TR output'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'tr_m_sync'
    , cmd  = 0x0D
    , docs = 'Synchronizes the metronome on the device #'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'tr_m_width'
    , cmd  = 0x0E
    , docs = 'Time for TR.PULSE as a percentage of TR.M'
    , args = { { 'port', u8 }
             , { 'width', s16 }
             }
    }
  , { name = 'tr_m_count'
    , cmd  = 0x0F
    , docs = 'Sets the number of repeats before deactivating (0=infinity)'
    , args = { { 'port', u8 }
             , { 'count', s16 }
             }
    }
  , { name = 'tr_m_mul'
    , cmd  = 0x17
    , docs = 'Multiplies the M rate on TR output; defaults to 1 - no multiplication'
    , args = { { 'port', u8 }
             , { 'mult', s16 }
             }
    }
  , { name = 'tr_pulse_mute'
    , cmd  = 0x16
    , docs = 'Mutes or un-mutes TR output; 1 (mute) or 0 (un-mute)'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }

-- TXo Control Voltage (CV) Basic Commands

  , { name = 'cv'
    , cmd  = 0x10
    , docs = 'Set port CV to volts (bipolar), following SLEW time'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'cv_slew'
    , cmd  = 0x12
    , docs = 'CV port slew time in ms'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'cv_set'
    , cmd  = 0x11
    , docs = 'Set CV port to volts (bipolar), ignoring SLEW time'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'cv_off'
    , cmd  = 0x15
    , docs = 'CV port offset, volts added at final stage'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }

-- TXo Control Voltage (CV) Extended Commands

  , { name = 'cv_slew_s'
    , cmd  = 0x13
    , docs = 'CV port slew time in s'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'cv_slew_m'
    , cmd  = 0x14
    , docs = 'CV port slew time in m'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }
  , { name = 'cv_qt'
    , cmd  = 0x30
    , docs = 'CV target quantized to current CV.SCALE'
    , args = { { 'port', u8 }
             , { 'qt', s16 }
             }
    }
  , { name = 'cv_qt_set'
    , cmd  = 0x31
    , docs = 'CV target quantized to current CV.SCALE, ignoring slew'
    , args = { { 'port', u8 }
             , { 'qt', s16 }
             }
    }
  , { name = 'cv_n'
    , cmd  = 0x32
    , docs = 'CV target note # α in current CV.SCALE'
    , args = { { 'port', u8 }
             , { 'note', s16 }
             }
    }
  , { name = 'cv_n_set'
    , cmd  = 0x33
    , docs = 'CV target note # α in current CV.SCALE, ignoring slew'
    , args = { { 'port', u8 }
             , { 'note', s16 }
             }
    }
  , { name = 'cv_scale'
    , cmd  = 0x34
    , docs = 'Select scale for individual CV output (see scales list)'
    , args = { { 'port', u8 }
             , { 'scale', s16 }
             }
    }
  , { name = 'cv_log'
    , cmd  = 0x35
    , docs = 'Translates the output for CV output x to logarithmic mode'
    , args = { { 'port', u8 }
             , { 'scale', s16 }
             }
    }

-- TXo Control Voltage (CV) Experimental Commands - Oscillator Functions

  , { name = 'osc'
    , cmd  = 0x40
    , docs = 'Targets oscillation (1v/oct translated)'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'osc_set'
    , cmd  = 0x41
    , docs = 'Sets oscillation, ignores OSC.SLEW'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'osc_qt'
    , cmd  = 0x42
    , docs = 'Targets oscillation (1v/oct translated); quantized to current OSC.SCALE'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'osc_qt_set'
    , cmd  = 0x43
    , docs = 'Sets oscillation in current OSC.SCALE; ignores OSC.SLEW'
    , args = { { 'port', u8 }
             , { 'volts', s16V }
             }
    }
  , { name = 'osc_n'
    , cmd  = 0x46
    , docs = 'Targets oscillation to note; quantized to current OSC.SCALE'
    , args = { { 'port', u8 }
             , { 'note', s16 }
             }
    }
  , { name = 'osc_n_set'
    , cmd  = 0x47
    , docs = 'Sets oscillation to note # in current OSC.SCALE; ignores OSC.SLEW'
    , args = { { 'port', u8 }
             , { 'note', s16 }
             }
    }
  , { name = 'osc_fq'
    , cmd  = 0x44
    , docs = 'Targets oscillation to frequency α in Hz'
    , args = { { 'port', u8 }
             , { 'fq', s16 }
             }
    }
  , { name = 'osc_fq_set'
    , cmd  = 0x45
    , docs = 'Sets oscillation to frequency α in Hz; ignores OSC.SLEW'
    , args = { { 'port', u8 }
             , { 'fq', s16 }
             }
    }
  , { name = 'osc_lfo'
    , cmd  = 0x48
    , docs = 'Targets oscillation to frequency α in mHz (millihertz: 10^-3 Hz)'
    , args = { { 'port', u8 }
             , { 'fq', s16 }
             }
    }
  , { name = 'osc_lfo_set'
    , cmd  = 0x49
    , docs = 'Sets oscillation to frequency α in mHz (millihertz: 10^-3 Hz); ignores OSC.SLEW'
    , args = { { 'port', u8 }
             , { 'fq', s16 }
             }
    }
  , { name = 'osc_wave'
    , cmd  = 0x4A
    , docs = 'Sets the waveform'
    , args = { { 'port', u8 }
             , { 'wave', s16 }
             }
    }
  , { name = 'osc_sync'
    , cmd  = 0x4B
    , docs = 'Resets the phase of the oscillator to zero'
    , args = { { 'port', u8 }
             , { 'sync', s16 }
             }
    }
  , { name = 'osc_phase'
    , cmd  = 0x53
    , docs = 'Sets the phase offset of the oscillator to α (0 to 16384) - range of one cycle'
    , args = { { 'port', u8 }
             , { 'phase', s16 }
             }
    }
  , { name = 'osc_width'
    , cmd  = 0x4C
    , docs = 'Sets the width of the pulse wave (3) to α (0 to 100)'
    , args = { { 'port', u8 }
             , { 'width', s16 }
             }
    }
  , { name = 'osc_rect'
    , cmd  = 0x4D
    , docs = 'Rectifies the polarity of the oscillator to α (-2 to 2)'
    , args = { { 'port', u8 }
             , { 'pol', s16 }
             }
    }
  , { name = 'osc_slew'
    , cmd  = 0x4F
    , docs = 'Sets the slew time for the oscillator (portamento) to α (milliseconds)'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'osc_slew_s'
    , cmd  = 0x50
    , docs = 'Sets the slew time for the oscillator (portamento) to α (seconds)'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'osc_slew_m'
    , cmd  = 0x51
    , docs = 'Sets the slew time for the oscillator (portamento) to α (minutes)'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }
  , { name = 'osc_scale'
    , cmd  = 0x4E
    , docs = 'Sets the quantization scale for the oscillator to scale # α'
    , args = { { 'port', u8 }
             , { 'scale', s16 }
             }
    }
  , { name = 'osc_cyc'
    , cmd  = 0x54
    , docs = 'Targets the cycle length for the oscillator to α (milliseconds)'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'osc_cyc_s'
    , cmd  = 0x55
    , docs = 'Targets the cycle length for the oscillator to α (seconds)'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'osc_cyc_m'
    , cmd  = 0x56
    , docs = 'Targets the cycle length for the oscillator to α (minutes)'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }
  , { name = 'osc_cyc_set'
    , cmd  = 0x57
    , docs = 'Sets the cycle length for the oscillator to α (milliseconds)'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'osc_cyc_s_set'
    , cmd  = 0x58
    , docs = 'Sets the cycle length for the oscillator to α (seconds)'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'osc_cyc_m_set'
    , cmd  = 0x59
    , docs = 'Sets the cycle length for the oscillator to α (minutes)'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }
  , { name = 'osc_ctr'
    , cmd  = 0x5A
    , docs = 'Centers the oscillation on CV output; values are bipolar (-16384 to +16383) and map to -10 to +10'
    , args = { { 'port', u8 }
             , { 'ctr', s16 }
             }
    }

-- TXo Control Voltage (CV) Experimental Commands - Envelope Generator

  , { name = 'env_act'
    , cmd  = 0x60
    , docs = 'Activates the envelope generator for the CV output; (0 = off; 1 = on)'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'env_att'
    , cmd  = 0x61
    , docs = 'Attack time for the envelope; α in milliseconds'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'env_att_s'
    , cmd  = 0x62
    , docs = 'Attack time for the envelope; α in seconds'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'env_att_m'
    , cmd  = 0x63
    , docs = 'Attack time for the envelope; α in minutes'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }
  , { name = 'env_dec'
    , cmd  = 0x64
    , docs = 'Decay time for the envelope; α in milliseconds'
    , args = { { 'port', u8 }
             , { 'ms', s16 }
             }
    }
  , { name = 'env_dec_s'
    , cmd  = 0x65
    , docs = 'Decay time for the envelope; α in seconds'
    , args = { { 'port', u8 }
             , { 's', s16 }
             }
    }
  , { name = 'env_dec_m'
    , cmd  = 0x66
    , docs = 'Decay time for the envelope; α in minutes'
    , args = { { 'port', u8 }
             , { 'm', s16 }
             }
    }
  , { name = 'env_trig'
    , cmd  = 0x67
    , docs = 'Triggers the envelope to play'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'env_eoc'
    , cmd  = 0x6B
    , docs = 'End of Cycle pulse'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'env_eor'
    , cmd  = 0x6A
    , docs = 'End of Rise pulse'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'env_loop'
    , cmd  = 0x6C
    , docs = 'End of Rise pulse'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }
  , { name = 'env'
    , cmd  = 0x6D
    , docs = 'Allows output to act as a gate between the 0 and 1 state. Changing this value from 0 to 1 causes the envelope to trigger the attack phase and hold at the peak CV value'
    , args = { { 'port', u8 }
             , { 'state', s16 }
             }
    }

-- TXo Global Commands

  , { name = 'kill'
    , cmd  = 0x20
    , docs = 'Cancels TR pulses and CV slews'
    , args = { 'port', u8 }
    }
  , { name = 'tr_init'
    , cmd  = 0x22
    , docs = 'Init tr'
    , args = { 'port', u8 }
    }
  , { name = 'cv_init'
    , cmd  = 0x23
    , docs = 'Init cv '
    , args = { 'port', u8 }
    }
  , { name = 'cv_calib'
    , cmd  = 0x6E
    , docs = 'Locks the current offset as a calibration offset and saves it to persist between power cycles for output.'
    , args = { 'port', u8 }
    }
  , { name = 'cv_reset'
    , cmd  = 0x6F
    , docs = 'Clears the calibration offset for output.'
    , args = { 'port', u8 }
    }
  , { name = 'init'
    , cmd  = 0x24
    , docs = 'Init txo'
    , args = { 'unit', u8 }
    }
  }
, pickle = -- zero-index the port & send to multiple devices for port >= 4
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
[[

uint8_t port = data[1] - 1;  // zero-index the port
data[1]   = port % 4;      // wrap command for subsequent devices
*address += port / 4;      // increment address for subsequent devices

]]
}
end
