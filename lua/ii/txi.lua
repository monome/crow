do return
{ module_name  = 'TXi'
, manufacturer = 'bpc'
, i2c_address  = 0x68
, lua_name     = 'txi'
, commands     =
  { { name = 'nop'
    , cmd  = 2
    , docs = 'place holder'
    , args = { { 'channel', s8 }
             , { 'state', s8 }
             }
    }
  }
, getters =
  { { name = 'in'
    , cmd  = 0x0
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in_quant'
    , cmd  = 0x8
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in_N'
    , cmd  = 0x10
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'param'
    , cmd  = 0x4
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  }
, pickle = -- before sending a command to txi:
--  zero-index the channel
--  mask & pack the channel into the command bytes lowest 2 bits
--  change byte_count to 1 as we're only sending the packed command byte
--  use the channel number to determine txi address
[[

uint8_t chan = data[1] - 1; // zero index
data[0] |= (chan & 0x3); // mask channel
*byte_count = 1; // packed into a single byte
*address += chan >> 2; // ascending vals increment address

]]
, unpickle = -- when receiving the value from txi:
--  mask the 2 lowest bits of the command byte to 0 to collate all chans
[[

*command &= ~0x3; // discard channel info on first 2 bits

]]
}
end
