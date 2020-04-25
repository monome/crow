--- cv input library
-- examples of how the user will interract with the cv input

local Input = {}
Input.__index = Input

Input.inputs = {1,2}

function Input.new( chan )
    local i = { channel    = chan
              , _mode      = 'none'
              , time       = 0.1
              , threshold  = 1.0
              , hysteresis = 0.1
              , direction  = 'both'
              , windows    = {}
              , notes      = {}
              , temp       = 12
              , scaling    = 1.0
        -- user-customizable events
              , stream     = function(value) _c.tell('stream',chan,value) end
              , change     = function(state) _c.tell('change',chan,state and 1 or 0) end
              , midi       = function(data) _c.tell('midi',table.unpack(data)) end
              , window     = function(ix, direction) get_cv(chan) end
              , scale      = function(s) _c.tell('scale',s.note) end
              }
    setmetatable( i, Input )
    Input.inputs[chan] = i -- save reference for callback engine
    return i
end

function Input:get_value()
    return io_get_input( self.channel )
end

function Input:set_mode( mode, ... )
    -- TODO short circuit these comparisons by only looking at first char
    local args = {...}
    if mode == 'stream' then
        self.time = args[1] or self.time
        set_input_stream( self.channel, self.time )
    elseif mode == 'change' then
        self.threshold  = args[1] or self.threshold
        self.hysteresis = args[2] or self.hysteresis
        self.direction  = args[3] or self.direction
        set_input_change( self.channel
                        , self.threshold
                        , self.hysteresis
                        , self.direction
                        )
    elseif mode == 'midi' then
        set_input_midi( self.channel )
    elseif mode == 'window' then
        self.windows    = args[1] or self.windows
        self.hysteresis = args[2] or self.hysteresis
        self.direction  = args[3] or self.direction
    elseif mode == 'scale' then
        self.notes   = args[1] or self.notes
        self.temp    = args[2] or self.temp
        self.scaling = args[3] or self.scaling
        set_input_scale( self.channel
                       , self.notes
                       , self.temp
                       , self.scaling
                       )
    else
        set_input_none( self.channel )
    end
    self._mode = mode
end

--- METAMETHODS
Input.__newindex = function(self, ix, val)
    if ix == 'mode' then
        self._mode = val
        Input.set_mode(self, self._mode)
    end
end

Input.__index = function(self, ix)
    if     ix == 'volts' then
        return Input.get_value(self)
    elseif ix == 'query' then
        return function() _c.tell('stream',self.channel,Input.get_value(self)) end
    elseif ix == 'mode'  then
        return function(...) Input.set_mode( self, ...) end
    end
end

Input.__call = function(self, ...)
    local args = {...}
    if #args == 0 then
        -- FIXME this should be removed? use .volts instead
        return Input.get_value(self)
    else -- table call
        local m = 0
        --if #args[1] == 0 then _ end -- implies empty table call
        for k,v in pairs( args[1] ) do
            if k == 'mode' then m = v end -- defer mode change after setting params
            self[k] = v
        end
        if m ~= 0 then self.mode = m end -- apply mode change
    end
end

setmetatable(Input, Input) -- capture the metamethods

-- callback
function stream_handler( chan, val ) Input.inputs[chan].stream( val ) end
function change_handler( chan, val ) Input.inputs[chan].change( val ~= 0 ) end
function midi_handler( ... ) d = {...}; Input.inputs[1].midi(d) end
function scale_handler(chan,i,o,n,v)
    --TODO build this table in C as it'll be faster?
    s={index=i, octave=o, note=n, volts=v}
    Input.inputs[chan].scale(s)
end

return Input
