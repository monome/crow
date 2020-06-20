--- high-resolution metro API
-- @module metro
-- @alias Metro_mt
--require 'norns'
--norns.version.metro = '0.0.3'

local Metro = {}
Metro.__index = Metro

Metro.num_metros = 8
Metro.num_script_metros = 8

Metro.metros = {}
Metro.available = {}
Metro.assigned = {}


--- init/assign
-- 'allocate' a metro (assigns unused id)
function Metro.init (arg, arg_time, arg_count)
    local event = 0
    local time  = arg_time or 1
    local count = arg_count or -1

    if type(arg) == 'table' then
        event = arg.event
        time  = arg.time or 1
        count = arg.count or -1
    else
        event = arg
    end

    local id = nil
    for i, val in pairs(Metro.available) do
       if val == true then
          id = i
          break
       end
    end
    if id ~= nil then
        Metro.assigned[id] = true
        Metro.available[id] = false
        local m = Metro.metros[id]
        m.event = event
        m.time  = time
        m.count = count
        return m
    end
    print('Metro.init: nothing available')
    return nil
end

function Metro.free(id)
    Metro.metros[id]:stop()
    Metro.available[id] = true
    Metro.assigned[id] = false
end

function Metro.free_all()
   for i=1,Metro.num_script_metros do
      Metro.free(i)
   end
end


--- constructor;
 -- @param id : identifier (integer)
function Metro.new(id)
    local m = {}
    m.props = {}
    m.props.id = id
    m.props.time = 1
    m.props.count = -1
    m.props.event = nil
    m.props.init_stage = 1
    setmetatable(m, Metro)
    return m
end

--- start a metro
-- @param time - (optional) time period between ticks (seconds.) by default, re-use the last period
-- @param count - (optional) number of ticks. infinite by default
-- @param stage - (optional) initial stage number (1-based.) 1 by default
function Metro:start(time, count, stage)
    local vargs = {}
    if time then self.props.time = time end
    if count then self.props.count = count end
    if stage then self.props.init_stage = stage end
    self.is_running = true
    metro_start( self.props.id
               , self.props.time
               , self.props.count
               , self.props.init_stage
               ) -- C function
end

--- stop a metro
function Metro:stop()
    metro_stop(self.props.id) -- C function
    self.is_running = false
end


Metro.__newindex = function(self, idx, val)
    if idx == 'time' then
        self.props.time = val
-- NB: metro time isn't applied until the next wakeup.
-- this is true even if you are setting time from the metro event;
-- metro has already gone to sleep when lua main thread gets
-- if you need a fully dynamic metro, re-schedule on the wakeup
        metro_set_time(self.props.id, self.props.time)
    elseif idx == 'count' then self.props.count = val
    elseif idx == 'init_stage' then self.props.init_stage = val
    else -- FIXME: dunno if this is even necessary / a good idea to allow
        rawset(self, idx, val)
    end
end

--- class custom .__index;
-- [] accessor returns one of the static metro objects
Metro.__index = function(self, idx)
    if type(idx) == 'number' then
        return Metro.metros[idx]
    elseif idx == 'start' then return Metro.start
    elseif idx == 'stop' then return Metro.stop
    elseif idx == 'init' then return Metro.init
    elseif idx == 'id' then return self.props.id
    elseif idx == 'count' then return self.props.count
    elseif idx == 'time' then return self.props.time
    elseif idx == 'init_stage' then return self.props.init_stage
    elseif self.props.idx then
        return self.props.idx
    else
        return rawget(self, idx)
    end
end

setmetatable(Metro, Metro)

---------------------
---- static initialization

--- initialize module data
for i=1,Metro.num_metros do
   Metro.metros[i] = Metro.new(i)
end

for i=1,Metro.num_script_metros do
   Metro.available[i] = true
   Metro.assigned[i] = false
end



--- Global Functions
-- @section globals

--- event on metro tick from C;
metro_handler = function(idx, stage)
   if Metro.metros[idx] then
      if Metro.metros[idx].event then
         Metro.metros[idx].event(stage)
      end
   end
end

return Metro
