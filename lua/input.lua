--- cv input library
-- examples of how the user will interract with the cv input

local Input = {}
Input.__index = Input

Input.inputs = {1,2}

function Input.new( chan )
    local i = { channel    = chan
              , _mode      = 'none'
              , time       = 0.1
              , threshold  = 0.5
              , hysteresis = 0.1
              , direction  = 'both'
              , windows    = {}
              , notes      = {}
              , tones      = 0
              , quants     = {}
              , ratios     = {}
        -- user-customizable events
              , stream     = function(value) get_cv(chan) end
              , change     = function(state) _crow.tell('change',chan,state) end
              , window     = function(ix, direction) get_cv(chan) end
              , scale      = function(octave, ix) get_cv(chan) end
              , quantize   = function(octave, ix) get_cv(chan) end
              , ji         = function(octave, ix) get_cv(chan) end
              }
    setmetatable( i, Input )
    Input.inputs[chan] = i -- save reference for callback engine
    return i
end

function Input:get_value()
    return io_get_input( self.channel )
end

-- FIXME this whole approach needs to be reconsidered.
-- really need to use userdata to allow transparent c<->lua data sharing.
-- basically the set of params being packed into set_input_mode should be
-- a userdata object. lua just sets the c struct members.
function Input:set_mode( mode, ... )
    -- TODO short circuit these comparisons by only looking at first char
    local args = {...}
    if mode == 'stream' then
        self.time = args[1] or self.time
        metro_start( self.channel - 2 -- -2 jump before metros
                   , self.time
                   , -1
                   , 0
                   ) -- C function
        --set_input_mode( self.channel, mode ) -- FIXME
    else
        metro_stop(self.channel - 2) -- C function, -2 jump before metros
        if mode == 'change' then
            self.threshold  = args[1] or self.threshold
            self.hysteresis = args[2] or self.hysteresis
            self.direction  = args[3] or self.direction
            -- FIXME
            set_input_mode( self.channel
                          , mode
                          , self.threshold
                          , self.hysteresis
                          , self.direction
                          )
        elseif mode == 'window' then
            self.windows    = args[1] or self.windows
            self.hysteresis = args[2] or self.hysteresis
            self.direction  = args[3] or self.direction
        elseif mode == 'scale' then
            self.notes = args[1] or self.notes
        elseif mode == 'quantize' then
            self.tones = args[1] or self.tones
            self.quants = args[2] or self.scale
        elseif mode == 'ji' then
            self.ratios = args[1]
        else

        end
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
    if     ix == 'value' then return Input.get_value(self)
    elseif ix == 'mode'  then
        return function(...) Input.set_mode( self, ...) end
    end
end

Input.__call = function(self, ...)
    local args = {...}
    if args == nil then
        return Input.get_value(self)
    else -- assume table
        for k,v in pairs( args ) do
            self.k = v
        end
    end
end

setmetatable(Input, Input) -- capture the metamethods

-- callback
function stream_handler( chan, val ) Input.inputs[chan].stream( val ) end
function change_handler( chan, val ) Input.inputs[chan].change( val ) end

print 'input loaded'

return Input