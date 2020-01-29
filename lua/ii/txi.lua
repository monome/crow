-- bitmask definitions for commands
local PARAM = 0x0
local IN    = 0x4

local QUANT = 0x8
local N     = 0x10

do return
{ module_name  = 'TXi'
, manufacturer = 'bpc'
, i2c_address  = {0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F}
, lua_name     = 'txi'
, getters =
  { { name = 'param'
    , cmd  = PARAM
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'param_quant'
    , cmd  = PARAM | QUANT
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'param_N'
    , cmd  = PARAM | N
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in'
    , cmd  = IN
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in_quant'
    , cmd  = IN | QUANT
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  , { name = 'in_N'
    , cmd  = IN | N
    , args = { 'channel', s8 }
    , retval = { 'volts', s16V }
    }
  }
, pickle = -- combine command & channel into a single byte & set address
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
[[

uint8_t chan = data[1] - 1;  // zero-index the channel
data[0] |= (chan & 0x3);     // mask channel
*byte_count = 1;             // packed into a single byte
*address += chan >> 2;       // ascending vals increment address

]]
, unpickle = -- using the same command to parse the response from any channel
-- void unpickle( uint8_t* address, uint8_t* command, uint8_t* data );
[[

*command &= ~0x3;  // use same command for all 4 channels (by discarding 2LSBs)

]]
}
end
