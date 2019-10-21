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
    self.exe     = {}
    self.hold    = false  -- is the slope trigger currently held high
    self.in_hold = false  -- is eval currently in a held construct
    self.locked  = false  -- flag to lockout bangs during lock{}
    self.retStk = {}
    self.pc = 1
    return self
end

local function set_action( self, exe )
    if type(exe) == 'function' then
        self.exe = {exe}
    else self.exe = exe end
    self.hold   = false
    self.locked = false
    self.retStk = {}
    self.pc = 1
end

-- FIXME why can't we use :method call on restart, release & step?
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
            Asl.restart(self)
        elseif dir == 'release' then
            self.hold = false
            Asl.release(self)
        elseif dir == 'step' then -- do nothing
        elseif dir == 'unlock' then self.locked = false
        elseif dir == 'nil' then self.hold = true -- simulating a nil call
        else print'ERROR unmatched action string'
        end
    elseif t == 'boolean' then
        self.hold = dir
        if not dir then
            Asl.release(self)
        end
    else self.hold = true
    end

    if not self.locked then
        if Asl.isOver(self) then Asl.restart(self) end
        Asl.step(self)
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
    if self.exe == nil then print'no asl active' return end

    local p = get_frame(self)[self.pc]
    if not p then
        if self:exit() then
            if self:nek() then print'layer doesnt exist'
            else self:step() end
        end
        return
    end

    local t = type(p)
    if t == 'function' then
        local wait = p(self)
        if self:nek() then print'last exit'
        elseif not wait then self:step()
        end
    elseif t == 'string' then
        if self:nek() then print'string exit'
        else self:step() end
    elseif t == 'table' then
        self:enter(p):step()
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
    if next(self.retStk) then -- return stack non empty
        self.pc = table.remove( self.retStk )
        return true
    end
end

function Asl:recur()
    self.pc = 0
end

function Asl:isOver()
    if #self.retStk == 0 and self.pc > #self.exe then
        return true
    end
end

function Asl:restart()
    self.retStk = {}
    self.pc = 1
end

function Asl:cleanup(str)
    if     str == 'unlock' then self.lock = false
    elseif str == 'unhold' then self.in_hold = false
    else return true
    end
end

-- TODO abstract out common parts from Asl:step
function Asl:release()
    while self.in_hold do
        local f = get_frame(self)
        if f then
            local p = f[self.pc]
            if type(p) == 'string' then
                self:cleanup(p)
            end
        end
        self:exit()
        self:nek()
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
    elseif ix == 'release' then return Asl.release
    elseif ix == 'restart' then return Asl.restart
    elseif ix == 'cleanup' then return Asl.cleanup
    elseif ix == 'isOver' then return Asl.cleanup
    end
end

setmetatable(Asl, Asl)

--------------------------------
-- low level ASL building blocks

function to( dest, time, shape )
    -- COMPILE TIME
    local d,t,s
    if type(dest) == 'table' then -- accept table syntax
        local tt = dest
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

function asl_prepend( fn_head, fns )
    table.insert( fns, 1, fn_head )
    return fns
end

function asl_append( fn_tail, fns )
    table.insert( fns, fn_tail )
    return fns
end

function asl_if( fn_to_bool, fns )
    return {asl_prepend(
              function(self)
                if not fn_to_bool(self) then self:exit() end
              end
            , fns )}
end

function loop( fns )
    return {asl_append( Asl.recur, fns )}
end

function asl_wrap( fn_head, fns, fn_tail )
    return asl_append( fn_tail
                     , asl_prepend( fn_head
                                  , fns ))
end

function asl_while( fn_to_bool, fns )
    return {asl_wrap( function(self)
                        if not fn_to_bool(self) then self:exit() end
                      end
                    , fns
                    , Asl.recur
                    )}
end

----------------------------
-- high level ASL constructs

function times( count, fns )
    local c
    return asl_while(
              function(self)
                  if not c then c = count end
                  if c <= 0 then c = nil
                  else c = c-1; return true end
              end
            , fns
            )
end

function lock( fns ) -- table -> table
    return{ asl_append( function(self) self.lock = true end
                      , fns )
          , 'unlock'
          }
end

function held( fns ) -- table -> table
    return{ asl_if( function(self) return self.hold end
                , asl_append( function(self) return 'wait' end
                  , asl_prepend( function(self) self.in_hold = true end
                    , fns )))
          , 'unhold'
          }
end

return Asl
