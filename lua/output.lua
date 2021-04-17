local Output = {}

function Output.new( chan )
    local o = { channel = chan
              , level   = 5.0
              , rate    = 1/chan
              , shape   = 'linear'
              , slew    = 0.0
              , _scale  = 'none'
              , ji      = false -- mark if .scale is in just intonation mode
              , asl     = asl.new( chan )
--              , trig    = { asl      = asl.new(k)
--                          , polarity = 1
--                          , time     = 1
--                          , level    = 5
--                          }
              }
    -- o.asl.describe = lfo( function() return o.rate  end
    --                   , function() return o.level end
    --                   )

    setmetatable( o, Output )

    return o
end

--- METAMETHODS
-- setters
Output.__newindex = function(self, ix, val)
    if ix == 'action' then
        self.asl:describe(val)
    elseif ix == 'volts' then
        self.asl:describe(to(val, self.slew, self.shape))
        -- self.asl.action = to(val, self.slew, self.shape)
        self.asl:action()
    elseif ix == 'done' then
        self.asl.done = val
    elseif ix == 'scale' then
        set_output_scale(self.channel, self.ji and just12(val) or val)
    elseif ix == 'dyn' then
        self.asl.dyn = val
    end
end

-- getters
Output.__index = function(self, ix)
    if     ix == 'action'  then return self.asl.action
    elseif ix == 'volts'   then return LL_get_state(self.channel)
    elseif ix == 'running' then return self.asl.running
    elseif ix == 'scale'   then return
        function(...)
            local args = {...}
            if type(args[2]) == 'string' then
                self.ji = true
                args[2] = 12 -- fake 12TET with floating point just notes
                args[1] = just12(args[1])
            else self.ji = false end
            set_output_scale(self.channel, table.unpack(args)) -- close over .channel
        end
    elseif ix == 'dyn' then return self.asl.dyn
    end
end

Output.__call = function(self, ...)
    self.asl:describe(...)
    self.asl:action()
end


setmetatable(Output, Output) -- capture the metamethods

return Output
