--- A Slope Language
-- declarative scripts for time-based event structures
-- scripts can have compile-time, or call-time behaviour and
-- query hardware states at the appropriate time

local Asl = {}
Asl.__index = Asl

function Asl.new(id)
    local slope = {}
    setmetatable( slope, Asl )
    if id == nil then print'asl needs id of output channel' end
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

-- use a metamethod so we can can *assign* myAsl:action = lfo()
-- but then *call* myAsl:action(high/low) 
-- need a 'proxy' metatable. see: https://www.lua.org/pil/13.4.4.html
function Asl:action( fn )
    self.co     = fn
    self.hold   = false
    self.locked = false
end


-- RUNTIME behaviour

function Asl:step()
    _,wait = coroutine.resume( self.co, self )
    if wait ~= 'wait' then self:step() end
end

function Asl:set_hold_state( state )
    self.hold = state
end

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
-- TODO should this take args in a table for curly braces
-- there's an inferred inference in how it's used
-- but it's not just a wrapped 'go_toward' as it abstracts
-- the id based on the asl in use
function toward( dest, time, shape )
    local d,t,s
    if type(dest) == 'table' then -- accept table syntax
        local tt = dest
        d,t,s = tt.dest or 'here', tt.time or 0, tt.shape or 'linear'
    else
        d,t,s = dest or 'here', time or 0, shape or 'linear'
    end
    return coroutine.create(function( self )
        while true do
            if d == 'here' then d = 1 end -- TODO d=1 should get 'here'
            LL_toward( self.id, d, t, s )
            coroutine.yield( (t ~= 0) and 'wait' or nil )
        end
    end)
end


-- WRAPPING functions
function seq_coroutines( self, fns )
    for i=1, #fns, 1 do
        local _,wait = coroutine.resume( fns[i], self )
        coroutine.yield( wait )
    end
end

function asl_if( fn_to_bool, fns )
    return coroutine.create(function( self )
        if fn_to_bool( self ) then
            seq_coroutines( self, fns )
        end
    end)
end

function asl_wrap( enter_fn, fns, exit_fn )
    return coroutine.create(function( self )
        enter_fn( self )
        seq_coroutines( self, fns )
        exit_fn( self )
    end)
end

-- first layer of abstraction over VM
function loop( fns )
    return coroutine.create(function( self )
        while true do
            seq_coroutines( self, fns )
        end
    end)
end

function lock( fns )
    return asl_wrap( function( self ) self.lock = true end
                   , fns
                   , function( self ) self.lock = false end
                   )
end

function held( aslT )
    return asl_wrap( function( self ) self.in_hold = true end
                   , asl_if( function( self ) return self.hold end
                           , aslT
                           )
                   , function( self ) self.in_hold = false end
                   )
end

function times( count, aslT )
    local n = count
    return asl_if( function( self ) n = n - 1
                                    return (n > -1) end
                 , aslT
                 )
end

print 'asl lib loaded'

return Asl
