--- midi devices
-- @module midi
-- @alias Midi
-- taken directly from norns midi.lua

local Midi = {}
Midi.__index = Midi

--- convert data (midi bytes) to msg.
-- @tparam table data :
-- @treturn table msg : midi message table, contents vary depending on message
function Midi.to_msg(data)
  local msg = {}
  -- note on
  if data[1] & 0xf0 == 0x90 then
    msg = {
      note = data[2],
      vel = data[3],
      ch = data[1] - 0x90 + 1
    }
    if data[3] > 0 then
      msg.type = 'note_on'
    elseif data[3] == 0 then -- if velocity is zero then send note off
      msg.type = 'note_off'
    end
  -- note off
  elseif data[1] & 0xf0 == 0x80 then
    msg = {
      type = 'note_off',
      note = data[2],
      vel = data[3],
      ch = data[1] - 0x80 + 1
    }
  -- cc
  elseif data[1] & 0xf0 == 0xb0 then
    msg = {
      type = 'cc',
      cc = data[2],
      val = data[3],
      ch = data[1] - 0xb0 + 1
    }
  -- pitchbend
  elseif data[1] & 0xf0 == 0xe0 then
    msg = {
      type = 'pitchbend',
      val = data[2] + (data[3] << 7),
      ch = data[1] - 0xe0 + 1
    }
  -- key pressure
  elseif data[1] & 0xf0 == 0xa0 then
    msg = {
      type = 'key_pressure',
      note = data[2],
      val = data[3],
      ch = data[1] - 0xa0 + 1
    }
  -- channel pressure
  elseif data[1] & 0xf0 == 0xd0 then
    msg = {
      type = 'channel_pressure',
      val = data[2],
      ch = data[1] - 0xd0 + 1
    }
  -- program change
  elseif data[1] & 0xf0 == 0xc0 then
    msg = {
      type = 'program_change',
      val = data[2],
      ch = data[1] - 0xc0 + 1
    }
  -- start
  elseif data[1] == 0xfa then
    msg.type = 'start'
  -- stop
  elseif data[1] == 0xfc then
     msg.type = 'stop'
  -- continue
  elseif data[1] == 0xfb then
    msg.type = 'continue'
  -- clock
  elseif data[1] == 0xf8 then
    msg.type = 'clock'
  -- song position pointer
  elseif data[1] == 0xf2 then
    msg = {
        type = 'song_position',
        lsb = data[2],
        msb = data[3]
    }
  -- song select
  elseif data[1] == 0xf3 then
    msg = {
        type = 'song_select',
        val = data[2]
    }
  -- everything else
  else
    msg = {
      type = 'other',
    }
  end
  return msg
end

return Midi
