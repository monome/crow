--- new metro lib which is just stubs
-- this will be implemented directly in C?

local Metro = {}

Metro.init = metro_init
Metro.free_all = metro_free_all

Metro.start = metro_start
Metro.stop = metro_stop
Metro.__newindex = metro__newindex
Metro.__index = metro__index
setmetatable(Metro, Metro)

function Metro.new(id)
	local m = {event=nil, lud=makelightuserdata(id)}
	setmetatable(m, Metro)
	return m
end

for i=1,Metro.num_metros do
   Metro.metros[i] = Metro.new(i)
end

-- return Metro





--- init/assign
-- 'allocate' a metro (assigns unused id)
Metro.init = metro_init
-- function Metro.init (arg, arg_time, arg_count)
-- 	return metro_init(arg,arg_time,arg_count)
-- end

--- start a metro
-- @param time - (optional) time period between ticks (seconds.) by default, re-use the last period
-- @param count - (optional) number of ticks. infinite by default
-- @param stage - (optional) initial stage number (1-based.) 1 by default
Metro.start = metro_start
-- function Metro:start(time, count, stage)
-- 	metro_start(self, time,count,stage)
-- end

--- stop a metro
Metro.stop = metro_stop
-- function Metro:stop()
-- 	metro_stop(self)
-- end

Metro.free_all = metro_free_all
-- function Metro.free_all()
-- 	metro_free_all()
-- end

Metro.__newindex = metro__newindex
-- Metro.__newindex = function(self, idx, val)
--     if idx == 'time' then
--         self.props.time = val
-- -- NB: metro time isn't applied until the next wakeup.
-- -- this is true even if you are setting time from the metro event;
-- -- metro has already gone to sleep when lua main thread gets
-- -- if you need a fully dynamic metro, re-schedule on the wakeup
--         metro_set_time(self.props.id, self.props.time)
--     elseif idx == 'count' then self.props.count = val
--     elseif idx == 'init_stage' then self.props.init_stage = val
--     else -- FIXME: dunno if this is even necessary / a good idea to allow
--         rawset(self, idx, val)
--     end
-- end

--- class custom .__index;
-- [] accessor returns one of the static metro objects
Metro.__index = metro__index
-- Metro.__index = function(self, idx)
--     if type(idx) == 'number' then
--         return Metro.metros[idx]
--     elseif idx == 'start' then return Metro.start
--     elseif idx == 'stop' then return Metro.stop
--     elseif idx == 'init' then return Metro.init
--     elseif idx == 'id' then return self.props.id
--     elseif idx == 'count' then return self.props.count
--     elseif idx == 'time' then return self.props.time
--     elseif idx == 'init_stage' then return self.props.init_stage
--     elseif self.props.idx then
--         return self.props.idx
--     else
--         return rawget(self, idx)
--     end
-- end

setmetatable(Metro, Metro)

return Metro