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
    asl.retStk = {}
    asl.pc = 1
    setmetatable( asl, Asl )
    return asl
end

function Asl:init() -- reset to defaults
    self.exe     = {}     -- a coroutine to be!
    self.hold    = false  -- is the slope trigger currently held high
    self.in_hold = false  -- is eval currently in a held construct
    self.locked  = false  -- flag to lockout bangs during lock{}
    self.retStk = {}
    self.pc = 1
    return self
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
    self.retStk = {}
    self.pc = 1
end

-- INTERPRETER

local function do_action( self, dir )
    local t = type(dir)
    if t == 'table' then
        set_action(self,dir) -- assign new action. dir is an ASL!
        self.hold = true           -- call it!
    elseif t == 'string' then
        if dir == '' or dir == 'start' then
            self.hold = true
        elseif dir == 'restart' or dir == 'attack' then
            self.hold = true
            self:restart()
        elseif dir == 'release' then
            self.hold = false
            self:release()
        elseif dir == 'step' then self:step()
        elseif dir == 'unlock' then self.locked = false
        else print'ERROR unmatched action string'
        end
    elseif t == 'bool' then
        if dir then
            self.retStk = {}
            self.pc = 1
        end
        self.hold = dir
    else self.hold = true
    end

    if not self.locked then self:step() end
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
        if self:nek() then print'last exit'
        elseif not wait then self:step() end
    else
        self:enter(x):step()
    end
end

--------------------------------
-- program counter manipulations

function Asl:nek()
    if self.pc then
        self.pc = self.pc + 1
    else return 'over' end
end

function Asl:enter( fns )
    table.insert( self.retStk, self.pc )
    self.pc = 1
    return self
end

function Asl:exit()
    self.pc = table.remove( self.retStk )
end

function Asl:recur()
    self.pc = 0
end

function Asl:restart()
    self.retStk = {}
    self.pc = 1
end

function Asl:release()
    if self.in_hold then
        --TODO this is incorrect. need to call 'exit' until we're outside?
        self.retStk = {}
        self.pc = 1
    end
end

--------------
-- metamethods

Asl.__newindex = function(self, ix, val)
    if ix == 'action' then set_action( self, val) end
end

Asl.__index = function(self, ix)
    if ix == 'action' then
        return function(self,a) do_action(self,a) end
    elseif ix == 'step' then return Asl.step
    elseif ix == 'init' then return Asl.init
    -- private fns below
    elseif ix == 'nek' then return Asl.nek
    elseif ix == 'enter' then return Asl.enter
    elseif ix == 'exit' then return Asl.exit
    elseif ix == 'recur' then return Asl.recur
    elseif ix == 'restart' then return Asl.restart
    end
end

setmetatable(Asl, Asl)

--------------------------------
-- low level ASL building blocks

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

function asl_if( fn_to_bool, fns )
    table.insert( fns, 1
        ,function(self)
            if not fn_to_bool() then self:exit() end
        end)
    table.insert( fns, Asl.exit )
    return fns
end

function asl_wrap( fn_head, fns, fn_tail )
    table.insert( fns, 1, fn_head )
    table.insert( fns, fn_tail )
    table.insert( fns, Asl.exit )
    return fns
end

----------------------------
-- high level ASL constructs

function loop( fns )
    table.insert( fns, Asl.recur )
    return fns
end

function times( count, fns )
    local c
    table.insert( fns, 1
        ,function(self)
            if not c then c = count end -- init c
            if c <= 0 then
                c = nil
                self:exit()
            else c = c-1 end
        end)
    table.insert( fns, Asl.recur )
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
