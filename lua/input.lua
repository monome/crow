--- cv input library
-- examples of how the user will interract with the cv input

local Input = {}
Input.__index = Input

function Input.new( chan )
    local i = { channel    = chan
              , mode       = 'none'
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
              , change     = function(state) get_cv(chan) end
              , window     = function(ix, direction) get_cv(chan) end
              , scale      = function(octave, ix) get_cv(chan) end
              , quantize   = function(octave, ix) get_cv(chan) end
              , ji         = function(octave, ix) get_cv(chan) end
              }
    setmetatable( i, Input )
    return i
end

function Input:getvalue()
    return io_get_input( self.channel )
end

--- METAMETHODS
Input.__index = function(self, ix)
    if ix == 'value' then return Input.get_value(self)
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

setmetatable(Input, Input) -- capture the __index and __newindex metamethods

function Input:mode( mode, ... )
    -- TODO short circuit these comparisons by only looking at first char
    local args = {...}
    if     mode == 'stream' then
        self.time = args[1] or self.time
    elseif mode == 'change' then
        self.threshold  = args[1] or self.threshold
        self.hysteresis = args[2] or self.hysteresis
        self.direction  = args[3] or self.direction
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
    elseif mode == 'none' then
    else
        print('unknown mode')
        return
    end
    self.mode = mode
    -- TODO call C layer to set DSP action & callback
    _set_input_mode( self.channel, self.mode )
end

return Input
