--- A Slope Language
-- declarative scripts for time-based event structures
-- scripts can have compile-time, or call-time behaviour and
-- query hardware states at the appropriate time

local Asl = {}
Asl.__index = Asl

function Asl.new(id)
    local asl = {}
    if id == nil then print'asl needs id of output channel' end
    asl.id      = id or 1  -- id defaults to 1
    asl.co      = {}       -- a coroutine to be!
    asl.hold    = false    -- is the slope trigger currently held high
    asl.in_hold = false    -- is eval currently in a held construct
    asl.locked  = false    -- flag to lockout bangs during lock{}
    asl.cc      = false    -- flag for whether coroutine can be continued
    setmetatable( asl, Asl )
    return asl
end

function Asl:init() -- reset to defaults
    self.co      = {}     -- a coroutine to be!
    self.hold    = false  -- is the slope trigger currently held high
    self.in_hold = false  -- is eval currently in a held construct
    self.locked  = false  -- flag to lockout bangs during lock{}
    self.cc      = false
    return self     -- functional style
end


-- COMPILER

-- user interacts with this via the 'action' metamethod
-- myAsl.action = <asl script>
-- this is a *private* method
-- FIXME should be local?
function Asl:set_action( co )
    if type(co) == 'table' then -- needs to be wrapped in a coroutine
        self.co = asl_coroutine( co )
    else
        self.co = co
    end
    self.hold   = false
    self.locked = false
    self.cc     = true -- ready to coroutine!
end

-- INTERPRETER

-- user interacts with this via the 'action' metamethod
-- myAsl:action() and optional state arg for 'held' interaction
-- this is a *private* method
-- FIXME should be local?
function Asl:do_action( dir )
    local t = type(dir)
    if t == 'table' or t == 'thread' then
        Asl.set_action(self,dir) -- assign new action. dir is an ASL!
        dir = true           -- call it!
    end
    dir = dir or true -- default to rising action
    self.hold = dir
    if self.co ~= nil then
        if self.locked ~= true then
            self.cc = true -- reactivate if finished
            -- TODO need to restart if true
            -- TODO jump to release if false
            self:step()
        end
    end
end

-- TODO
-- Init can check if 'callback' is set to nil to know whether an ASL
-- callback is provided
-- If no callback, ASL can create it's own timer & assume actions are
-- completed on time

function Asl:step()
    if     self.cc == false then print'done' return
    elseif self.co == nil then print'no slope' return end
    local _,wait = coroutine.resume( self.co, self )
    if     wait == 'exit' then self.cc = false
    elseif wait == nil then self:step() end
end

--- METAMETHODS
-- asign to the member 'action' to compile an asl script to a coroutine
Asl.__newindex = function(self, ix, val)
    if ix == 'action' then Asl.set_action( self, val) end
end

-- call the member 'action' to start the asl coroutine
--   if called w a table arg, treat it as a new ASL to run immediately
Asl.__index = function(self, ix)
    if ix == 'action' then
        return function(self,a) Asl.do_action(self,a) end
    elseif ix == 'step' then return Asl.step
    elseif ix == 'init' then return Asl.init end
end

setmetatable(Asl, Asl) -- capture the __index and __newindex metamethods

-- RUNTIME behaviour

-- hw access fn
    -- TODO consider that toward with missing args should insert closured
    -- queries to the self.params so that they automatically reflect global
    -- settings!!!!!

function toward( dest, time, shape )
    local d,t,s
    if type(dest) == 'table' then -- accept table syntax
        local tt = dest
        -- provide 'delay' and 'now' methods
        if tt.delay then d,t,s = 'here', tt.delay, 'linear'
        elseif tt.now then d,t,s = tt.now, 0, 'linear' end
        d,t,s = tt.dest or 'here', tt.time or 0, tt.shape or 'linear'
    else
        d,t,s = dest or 'here', time or 0, shape or 'linear'
    end
    return coroutine.create(function( self )
        while true do
            if d == 'here' then d = LL_get_state( self.id ) end
            LL_toward( self.id, d, t, s )
            coroutine.yield( (t ~= 0) and 'wait' or nil )
        end
    end)
end


-- WRAPPING functions
function seq_coroutines( self, fns, priority )
    for i=1, #fns do
        repeat
            local _,wait,greedy = coroutine.resume( fns[i], self )
            coroutine.yield( (wait~='exit') and wait or nil, priority )
        until( not greedy or priority > greedy )
    end
end

-- TODO does this need diff seq handling?
function asl_coroutine( fns )
    return coroutine.create(function( self )
        while true do
            seq_coroutines( self, fns, 1 )
            coroutine.yield('exit')
        end
    end)
end

function asl_if( fn_to_bool, fns )
    return coroutine.create(function( self )
        while true do
            if fn_to_bool( self ) then
                seq_coroutines( self, fns, 1 )
            end
            coroutine.yield('exit')
        end
    end)
end

function asl_wrap( enter_fn, fns, exit_fn )
    return coroutine.create(function( self )
        while true do
            enter_fn( self )
            seq_coroutines( self, fns, 1 )
            exit_fn( self )
            coroutine.yield('exit')
        end
    end)
end

-- first layer of abstraction over VM
function loop( fns )
    return coroutine.create(function( self )
        while true do
            seq_coroutines( self, fns, 1 )
        end
    end)
end

function weave( fns )
    return coroutine.create(function( self )
        while true do
            seq_coroutines( self, fns, 2 )
        end
    end)
end

function lock( fns )
    return asl_wrap( function( self ) self.lock = true end
                   , fns
                   , function( self ) self.lock = false end
                   )
end

function held( fns )
    return asl_wrap( function( self ) self.in_hold = true end
                   , asl_if( function( self ) return self.hold end
                           , fns
                           )
                   , function( self ) self.in_hold = false end
                   )
end

function times( count, fns )
    return coroutine.create(function( self )
        while true do
            local n = count
            while n > 0 do
                n = n - 1
                seq_coroutines( self, fns, 1 )
            end
            coroutine.yield('exit')
        end
    end)
end

print 'asl loaded'

return Asl
