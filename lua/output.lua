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
              , done    = function() end -- customizable event called on asl completion
              , clock_div = 1
              , ckcoro  = false -- clock coroutine
              }
    return setmetatable( o, Output )
end

function Output.clock(self, div)
    if type(div) == 'string' then -- 'off' or 'none' will cancel a running output clock
        if self.ckcoro then clock.cancel(self.ckcoro) end
        return
    end
    self.clock_div = div or self.clock_div
    self.asl:describe(pulse())
    if self.ckcoro then clock.cancel(self.ckcoro) end -- replace the existing coro
    self.ckcoro = clock.run(function()
            while true do
                clock.sync(self.clock_div)
                self.asl:action()
            end
        end)
end

--- METAMETHODS
-- setters
Output.__newindex = function(self, ix, val)
    if ix == 'action' then
        self.asl:describe(val)
    elseif ix == 'volts' then
        self.asl:describe(to(val, self.slew, self.shape))
        self.asl:action()
    elseif ix == 'scale' then
        set_output_scale(self.channel, self.ji and just12(val) or val)
    end
end

-- getters
Output.__index = function(self, ix)
    if     ix == 'action'  then return self.asl.action
    elseif ix == 'volts'   then return LL_get_state(self.channel)
    elseif ix == 'clock'   then return Output.clock
    elseif ix == 'scale'   then return
        function(...) -- return lambda as we're closing over self
            local args = {...}
            if type(args[2]) == 'string' then
                self.ji = true
                args[2] = 12 -- fake 12TET with floating point just notes
                args[1] = just12(args[1])
            else self.ji = false end
            set_output_scale(self.channel, table.unpack(args))
        end
    elseif ix == 'dyn' then return self.asl.dyn
    end
end

Output.__call = function(self, arg)
    if type(arg) == 'table' then -- being passed a literal asl to interpret & begin
        self.asl:describe(arg)
        self.asl:action()
    else self.asl:action(arg) end -- args are forwarded as an action
end


setmetatable(Output, Output) -- capture the metamethods

return Output
