local Output = {}

function Output.new( chan )
    local o = { channel = chan
              , level   = 5.0
              , rate    = 1/chan
              , shape   = 'linear'
              , slew    = 0.0
              , asl     = Asl.new( chan )
--              , trig    = { asl      = Asl.new(k)
--                          , polarity = 1
--                          , time     = 1
--                          , level    = 5
--                          }
              }
    o.asl.action = lfo( function() return o.rate  end
                      , function() return o.shape end
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
        self.asl.action = {to(val, self.slew)}
        self.asl:action()
    end
end

-- getters
Output.__index = function(self, ix)
    if ix == 'action' then
        return self.asl.action
    elseif ix == 'volts' then
        return LL_get_state(self.channel)
    end
end

Output.__call = function(self, ...)
    local args = {...}
    if #args == 0 then
        self.asl:action()
    else -- table call
        self.asl.action = args[1]
        self.asl:action()
        --local m = 0
        ----if #args[1] == 0 then _ end -- implies empty table call
        --for k,v in pairs( args[1] ) do
        --    if k == 'mode' then m = v end -- defer mode change after setting params
        --    self[k] = v
        --end
        --if m ~= 0 then self.mode = m end -- apply mode change
    end
end

setmetatable(Output, Output) -- capture the metamethods

return Output
