local Output = {}

function Output.new( chan )
    local o = { channel = chan
              , level   = 5.0
              , rate    = 1/chan
              , shape   = 'linear'
              , slew    = 0.0
              , _asl    = {}
              , ref     = {}
              }
    setmetatable( o, Output )

    o.asl = asl.new(chan) -- force assignment to hardware channels
    o.asl.action = lfo( function() return o.rate  end
                      , function() return o.level end
                      )
    return o
end

--- METAMETHODS
-- setters
Output.__newindex = function(self, ix, val)
    if ix == 'asl' then
        self._asl = val -- capture the new ASL object
        -- TODO tell slope library to attach dsp output to HW output
        self.ref = self._asl.ref -- capture ASL object as output reference
            -- NB: this will be different reference when synth mode added
    elseif ix == 'action' then
        self._asl.action = val
    elseif ix == 'volts' then
        self._asl.action = to(val, self.slew, self.shape)
        self._asl:action()
    elseif ix == 'done' then
        self._asl.done = val
    end
end

-- getters
Output.__index = function(self, ix)
    if     ix == 'asl'     then return self._asl
    elseif ix == 'action'  then return self._asl.action
    elseif ix == 'volts'   then return get_state(self.channel)
    elseif ix == 'running' then return self._asl.running
    end
end

Output.__call = function(self, ...)
    self._asl:action(...)
end


setmetatable(Output, Output) -- capture the metamethods

return Output
