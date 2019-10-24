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
    asl.running = false    -- flag to mark if asl is active
    asl.retStk = {}
    asl.pc = 1
    asl.done = function() end -- function that does nothing
    setmetatable( asl, Asl )
    return asl
end

function Asl:init() -- reset to defaults
    self.exe     = {}
    self.hold    = false  -- is the slope trigger currently held high
    self.in_hold = false  -- is eval currently in a held construct
    self.locked  = false  -- flag to lockout bangs during lock{}
    self.running = false  -- flag to mark if asl is active
    self.retStk = {}
    self.pc = 1
    return self
end


-------------------------------
-- setting & calling .action

local function set_action( self, exe )
    if type(exe) == 'function' then
        self.exe = {exe}
    else self.exe = exe end
    self.hold   = false
    self.locked = false
    self.retStk = {}
    self.pc = 1
end

local function do_action( self, dir )
    self.running = true  -- mark asl as running
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


-------------------------------------
-- metamethods for handling .action

Asl.__newindex = function(self, ix, val)
    if ix == 'action' then set_action( self, val) end
end

Asl.__index = function(self, ix)
    if ix == 'action' then
        return function(self,a) do_action(self,a) end
    elseif ix == 'step' then return Asl.step
    elseif ix == 'init' then return Asl.init
    -- private fns below (called from step)
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


--------------------------------------------
-- program counter (runtime) manipulations

local function get_frame( self )
    local f = self.exe
    for i=1, #self.retStk do
        f = f[self.retStk[i]]
    end
    return f
end

-- Asl:step() is the primary entry point for a to-callback
function Asl:step()
    if self.exe == nil then print'no asl active' return end

    -- get the new frame
    -- escape up a layer if it doesn't exist & recurse
    local p = get_frame(self)[self.pc]
    if not p then
        if self:exit() then
            if self:nek() then print'layer doesnt exist'
            else self:step() end
        else
            self.running = false
            self.done() -- user callback on completion
        end
        return
    end

    -- execute the next element in the asl table
    local t = type(p)
    if t == 'function' then
        -- run a function & recurse until seeing a 'wait'
        -- 'wait' is typically the result of a call to 'to'
        local wait = p(self)
        if self:nek() then print'last exit'
        elseif not wait then self:step()
        end
    elseif t == 'string' then
        -- skip strings (they are comments or tags)
        if self:nek() then print'string exit'
        else self:step() end
    elseif t == 'table' then
        -- enter a table which should contain another asl
        self:enter(p):step()
    end
end

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


----------------------------
-- low-level ASL constructs

function Asl.prepend( fn_head, fns )
    table.insert( fns, 1, fn_head )
    return fns
end

function Asl.append( fn_tail, fns )
    table.insert( fns, fn_tail )
    return fns
end

function Asl._if( fn_to_bool, fns )
    return {Asl.prepend(
              function(self)
                if not fn_to_bool(self) then self:exit() end
              end
            , fns )}
end

function Asl.wrap( fn_head, fns, fn_tail )
    return Asl.append( fn_tail
                     , Asl.prepend( fn_head
                                  , fns ))
end

function Asl._while( fn_to_bool, fns )
    return {Asl.wrap( function(self)
                        if not fn_to_bool(self) then self:exit() end
                      end
                    , fns
                    , Asl.recur
                    )}
end


-----------------------------------------------
-- helper for deferring computation to runtime

function Asl.runtime(fn,...)
    local args = {...}
    if #args == 0 then
        return fn
    else
        local a = table.remove(args,1)
        if type(a) == 'function' then
            return Asl.runtime(function(...) return fn(a(),...) end, table.unpack(args))
        else
            return Asl.runtime(function(...) return fn(a,...) end, table.unpack(args))
        end
    end
end


-------------------------------------
-- public (global) ASL constructs

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

function loop( fns )
    return {Asl.append( Asl.recur, fns )}
end

function times( count, fns )
    local c
    return Asl._while(
              function(self)
                  if not c then c = count end
                  if c <= 0 then c = nil
                  else c = c-1; return true end
              end
            , fns
            )
end

function lock( fns ) -- table -> table
    return{ Asl.append( function(self) self.lock = true end
                      , fns )
          , 'unlock'
          }
end

function held( fns ) -- table -> table
    return{ Asl._if( function(self) return self.hold end
                , Asl.append( function(self) return 'wait' end
                  , Asl.prepend( function(self) self.in_hold = true end
                    , fns )))
          , 'unhold'
          }
end


---------------------------------------------------------
-- capture all the metamethods & Asl namespace functions

setmetatable(Asl, Asl)


return Asl
