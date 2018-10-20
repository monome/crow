--- A Slope Language
-- declarative scripts for time-based event structures
-- scripts can have compile-time, or call-time behaviour and
-- query hardware states at the appropriate time
--
--
-- rewrite using coroutine approach for the execution of the ASL

local table = require('table')
local print = print

local Asl = {}
Asl.__index = Asl

function Asl.new(id)
    local slope = {}
    setmetatable(slope, Asl)
    if id == nil then print('asl needs id of output channel') end
    slope.id = id or 1
    slope:init()
    return slope
end

function Asl:init() -- reset to defaults
    self.co      = nil    -- a coroutine to be!
    self.hold    = false  -- is the slope trigger currently held high
    self.in_hold = false  -- is eval currently in a held construct
    self.locked  = false  -- flag to lockout bangs during lock{}
end

-- COMPILER
-- create a table with a sequence of functions

-- use a metamethod so we can can *assign* myAsl:action = lfo()
-- but then *call* myAsl:action(high/low) 
-- need a 'proxy' metatable. see: https://www.lua.org/pil/13.4.4.html
function Asl:action( fn )
    self.co     = fn
    self.hold   = false
    self.locked = false
end


-- RUNTIME behaviour
-- relates bang() to internal AST representation

-- step through the program recursively!
-- this is stupid. we're searching the tree every step!
-- think about how to do this in a direct way

function Asl:step()
    _,wait = coroutine.resume(self.co, self)
    if wait ~= 'wait' then self:step() end
end

function Asl:set_hold_state(state)
    self.hold = state
end

-- public accessors
function Asl:bang( dir )
    self.hold = dir
    if self.co ~= nil then
        if self.locked ~= true then
            --TODO release handling
            self:step()
        end
    end
end

-- hw access fn
function toward( dest, time, shape )
    local d,t,s = dest or 0, time or 1, shape or 'linear'
    return coroutine.create(function(self)
        while true do
            LL_toward(self,d,t,s)
            coroutine.yield( (t ~= 0) and 'wait' or nil )
        end
    end)
end

-- sugared toward(). could be variable arity instead?
function now( dest )
    return toward( dest, 0, LINEAR )
end

function delay( time )
    --TODO here needs to call self to find current state
    local here = 0 -- can this be a metamethod call?
    return toward( here, time, LINEAR )
end

-- WRAPPING functions
function asl_if( fn_to_bool, fns )
    return coroutine.create(function(self)
        if fn_to_bool(self) then
            for i=1, #fns, 1 do
                _,wait = coroutine.resume(fns[i], self)
                coroutine.yield(wait)
            end
        end
    end)
end

function asl_wrap( enter_fn, fns, exit_fn )
    return coroutine.create(function(self)
        enter_fn(self)
        for i=1, #fns, 1 do
            _,wait = coroutine.resume(fns[i], self)
            coroutine.yield(wait)
        end
        exit_fn(self)
    end)
end

-- first layer of abstraction over VM
function loop( fns )
    return coroutine.create(function(self)
        while true do
            for i=1,#fns,1 do
                _,wait = coroutine.resume(fns[i], self)
                coroutine.yield(wait)
            end
        end
    end)
end

-- could abstract this form into asl_wrap()
function lock( fns )
    return asl_wrap( function(self) self.lock = true end
                   , fns
                   , function(self) self.lock = false end
                   )
end

function held( aslT )
    return asl_wrap( function(self) self.in_hold = true end
                   , asl_if( function(self) return self.hold end
                           , aslT
                           )
                   , function(self) self.in_hold = false end
                   )
end

function times( count, aslT )
    local n = count
    return asl_if( function(self) n = n - 1
                                  return (n > -1) end
                 , aslT
                 )
end

print 'asl lib loaded'

return Asl
