local Output = {}

function Output.new( chan )
    local o = { channel = chan
              , level   = 5.0
              , rate    = 1/chan
              , shape   = 'linear'
              , slew    = 0.0
              , asl     = asl.new( chan )
--              , trig    = { asl      = asl.new(k)
--                          , polarity = 1
--                          , time     = 1
--                          , level    = 5
--                          }
              }
    o.asl.action = lfo( function() return o.rate  end
                      , function() return o.level end
                      )

    setmetatable( o, Output )

    return o
end

--- METAMETHODS
-- setters
Output.__newindex = function(self, ix, val)
    if ix == 'action' then
        self.asl.action = val
    elseif ix == 'volts' then
        self.asl.action = {to(val, self.slew, self.shape)}
        self.asl:action()
    elseif ix == 'done' then
        self.asl.done = val
    end
end

-- getters
Output.__index = function(self, ix)
    if     ix == 'action'  then return self.asl.action
    elseif ix == 'volts'   then return LL_get_state(self.channel)
    elseif ix == 'running' then return self.asl.running
    end
end

Output.__call = function(self, ...)
    self.asl:action(...)
end


setmetatable(Output, Output) -- capture the metamethods

return Output
