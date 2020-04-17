--- A Slope Language
-- declarative scripts for time-based event structures
-- scripts can have compile-time, or call-time behaviour and
-- query hardware states at the appropriate time

local Asl = {}
Asl.__index = Asl

-- Manage dynamic allocation & reference-to-id matching
Asl.LIMIT = 4    -- MUST == SLOPE_CHANNELS (in lib/io.h)
Asl.list    = {} -- Objects by id
Asl.refs    = {} -- Objects by reference
Asl.open_id = {} -- next to be assigned
for n=1,Asl.LIMIT do
    Asl.list[n] = {}
    Asl.refs[n] = {}
    table.insert( Asl.open_id, n )
end

function Asl.new(id)
    if not id then -- dynamically assign an ASL
        if #Asl.open_id > 0 then           -- check for a free ASL
            id = Asl.open_id[1]            -- copy it to id
            table.remove( Asl.open_id, 1 ) -- rm from the dynamic list
        else
            print 'All ASLs already in use'
            return
        end
    else -- overwrite a specific index
        if id < 1 or id > Asl.LIMIT then
            print 'ASL index out of range'
            return
        else
            for k,v in ipairs( Asl.open_id ) do
                if v == id then table.remove( Asl.open_id, k ) end
                break
            end
        end
    end

    local a       = {}
    local ref     = new_slope(id) -- get slope address from C-lib
    Asl.list[id]  = a
    Asl.refs[ref] = a

    a.ref     = ref   -- C pointer to the Slope object TODO not needed
    a.exe     = {}    -- an ASL to be
    a.hold    = false -- is the slope trigger currently held high
    a.in_hold = false -- is eval currently in a held construct
    a.locked  = false -- flag to lockout bangs during lock{}
    a.running = false
    a.retStk  = {}
    a.pc      = 1
    a.done    = function() end -- function that does nothing
    setmetatable( a, Asl )
    return a
end

-- FIXME never called, use the __gc metamethod
function Asl.free(id)
    -- TODO stop ASL
    -- TODO deactivate C layer
    --Asl.list.id = nil -- TODO switch to ref-based action
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
    if t == 'table' or t == 'function' then
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

Asl.__call = function(self, ...)
    do_action(self, ...)
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


---------------------------------------------------------
-- capture all the metamethods & Asl namespace functions

setmetatable(Asl, Asl)


-------------------------------------
-- global ASL constructs

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
        while type(d) == 'function' do d = d() end
        while type(t) == 'function' do t = t() end
        while type(s) == 'function' do s = s() end
        go_toward( self.ref, d, t, s ) -- go_toward takes number or 'here' for *d*
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
-- public event called from C event queue

function asl_handler( ref )
    Asl.refs[ref]:step()
end


return Asl
