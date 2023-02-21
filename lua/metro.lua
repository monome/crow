--- new metro lib which is just stubs
-- this will be implemented directly in C?

local Metro = {}

Metro.num_metros = 8

Metro.init = l_metro_init
Metro.free_all = l_metro_free_all
Metro.start = l_metro_start
Metro.stop = l_metro_stop
Metro.__newindex = l_metro__newindex
Metro.__index = l_metro__index

function Metro.new(id)
  local m = {event=false, id=id} -- id becomes a lightuserdatum pointer
  return setmetatable(m, Metro)
end

setmetatable(Metro, Metro)

for i=1,Metro.num_metros do
   rawset(Metro, i, Metro.new(i))
end

return Metro
