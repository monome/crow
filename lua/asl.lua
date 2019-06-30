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
    asl.exe     = {}       -- a coroutine to be!
    asl.hold    = false    -- is the slope trigger currently held high
    asl.in_hold = false    -- is eval currently in a held construct
    asl.locked  = false    -- flag to lockout bangs during lock{}
    asl.cc      = false    -- flag for whether coroutine can be continued
    asl.retStk = {}
    asl.pc = 0
    setmetatable( asl, Asl )
    return asl
end

function Asl:init() -- reset to defaults
    self.exe     = {}     -- a coroutine to be!
    self.hold    = false  -- is the slope trigger currently held high
    self.in_hold = false  -- is eval currently in a held construct
    self.locked  = false  -- flag to lockout bangs during lock{}
    self.cc      = false
    self.retStk = {}
    self.pc = 0
    return self     -- functional style
end


-- COMPILER

-- user interacts with this via the 'action' metamethod
-- myAsl.action = <asl script>
local function set_action( self, exe )
    if type(exe) == 'function' then
        self.exe = {exe}
    else self.exe = exe end
    self.hold   = false
    self.locked = false
    self.cc     = true -- ready to exec!
    self.retStk = {}
    self.pc = 1
end

-- INTERPRETER

-- user interacts with this via the 'action' metamethod
-- myAsl:action() and optional state arg for 'held' interaction
-- this is a *private* method
local function do_action( self, dir )
    local t = type(dir)
    if t == 'table' then
        set_action(self,dir) -- assign new action. dir is an ASL!
        dir = true           -- call it!
    elseif dir == 'restart' then
        self.retStk = {}
        self.pc = 1
    end
    dir = dir or true -- default to rising action
    self.hold = dir
    if self.exe ~= nil then
        if self.locked ~= true then
            self.cc = true -- reactivate if finished
            -- TODO need to restart if true
            -- TODO jump to release if false
            self:step()
        end
    end
end

local function get_frame( self )
    local f = self.exe
    for i=1, #self.retStk do
        f = f[self.retStk[i]]
    end
    return f
end

function Asl:step()
    if self.exe == nil then print'no slope' return end

    local x = get_frame(self)[self.pc]
    if not x then print'over' return
    elseif type(x) == 'function' then
        local wait = x(self)
        if nek(self) then print'last exit'
        elseif not wait then self:step() end
    else
        enter(self,x):step()
    end
end

--- METAMETHODS
-- asign to the member 'action' to compile an asl script to a coroutine
Asl.__newindex = function(self, ix, val)
    if ix == 'action' then set_action( self, val) end
end

-- call the member 'action' to start the asl coroutine
--   if called w a table arg, treat it as a new ASL to run immediately
Asl.__index = function(self, ix)
    if ix == 'action' then
        return function(self,a) do_action(self,a) end
    elseif ix == 'step' then return Asl.step
    elseif ix == 'init' then return Asl.init
    end
end

setmetatable(Asl, Asl) -- capture the __index and __newindex metamethods

function toward( dest, time, shape )
    -- COMPILE TIME
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

    -- RUNTIME
    return function( self )
        LL_toward( self.id
                 , (d == 'here') and LL_get_state( self.id ) or d
                 , t
                 , s
                 )
        return (t ~= 0)
    end
end

--------------------------------
-- program counter manipulations

function nek( self )
    if self.pc then
        self.pc = self.pc + 1
    else return 'over' end
end

function enter( self, fns )
    table.insert( self.retStk, self.pc )
    self.pc = 1
    return self
end

function exit( self )
    self.pc = table.remove( self.retStk )
end

function recur( self )
    self.pc = 0
end

--------------------------------
-- low level ASL building blocks

function asl_if( fn_to_bool, fns )
    table.insert( fns, 1, function(self) if not fn_to_bool() then exit(self) end end )
    table.insert( fns, exit )
    return fns
end

function asl_wrap( startfn, fns, exitfn )
    table.insert( fns, 1, startfn )
    table.insert( fns, exitfn )
    table.insert( fns, exit )
    return fns
end

----------------------------
-- high level ASL constructs

function loop( fns )
    table.insert( fns, recur )
    return fns
end

function times( count, fns )
    local c
    table.insert( fns, 1
        ,function(self)
            if not c then c = count end -- init c
            if c <= 0 then
                c = nil
                exit(self)
            else c = c-1 end
        end)
    table.insert( fns, recur )
    return fns
end

function lock( fns )
    return asl_wrap( function(self) self.lock = true end
                   , fns
                   , function(self) self.lock = false end
                   )
end

function held( fns )
    return asl_wrap( function(self) self.in_hold = true end
                   , asl_if( function(self) return self.hold end
                           , fns
                           )
                   , function(self) self.in_hold = false end
                   )
end

print 'asl loaded'

return Asl
