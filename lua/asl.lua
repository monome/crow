local Asl = {}

local Dynmt = {
    __newindex = function(self, k, v) casl_setdynamic(self.id, self._names[k], v) end,
    __index = function(self, k) return casl_getdynamic(self.id, self._names[k]) end
}

function Asl.new(id)
    local c = {id = id or 1}
    c.dyn = setmetatable({_names={}, id=c.id}, Dynmt) -- needs link to `id`
    setmetatable(c, Asl)
    return c
end

-- returns a copy of the table, so a description can be re-used
function Asl.link(self, tab)
    local t2 = {}
    for k,v in pairs(tab) do
        local typ = type(v)
        if typ == 'function' then
            return v(self, tab[2]) -- call compiler fn with self & return new table
            -- early return bc fn must be in the first position, and consumes following arg
        elseif typ == 'table' then
            t2[k] = Asl.link(self, v) -- link nested tables (won't copy unchanged tables)
        else
            t2[k] = v -- copy value into new table
        end
    end
    return t2
end

function Asl:describe(d)
    casl_cleardynamics(self.id)
    self.dyn._names = {} -- clear local dynamic refs
    casl_describe(self.id, Asl.link(self, d))
end

function Asl.set_held(self, b)
    if self.dyn._held then
        self.dyn._held = b
        return true -- ie. success in setting
    end
end

-- direc(tive) can take 0/1 or false/true. ONLY has effect if there is a held{} construct
-- truthy always restarts
-- falsey means 'release'
function Asl:action(direc)
    if direc == nil then -- no arg is always 'restart'
        casl_action(self.id, 1)
    elseif direc == 'unlock' then casl_action(self.id, 2) -- release lock construct
    else -- set `held` dyn if it exists. call action unless no `held` and direc is falsey
        local s = (direc == true or direc == 1) and 1 or 0
        if Asl.set_held(self, s) or s==1 then casl_action(self.id, s) end
    end
end


function Asl.dyn_compiler(self, d)
    -- register a dynamic pair {name=default}, and return a reference to it
    local elem, typ = d, 'DYN'
    if d[1] == 'NMUT' then elem, typ = d[2], 'NMUT' end -- unwrap named mutables
    local k,v = next(elem)
    if not self.dyn._names[k] then -- check if it's a new entry
        self.dyn._names[k] = casl_defdynamic(self.id)
    end
    local ref = self.dyn._names[k]
    self.dyn[k] = v -- set the default
    return {typ, ref}
end


-- metatables
Asl.__index = function(self, ix)
    if     ix == 'describe' then return Asl.describe
    elseif ix == 'action' then return Asl.action
    end
end
setmetatable(Asl, Asl)


-- basic constructs
function to(volts, time, shape)
    return {'TO', volts or 0.0, time or 1.0, shape or 'linear'}
end

function loop(t)
    table.insert(t,{'RECUR'})
    return t
end

function Asl._if(pred, t)
    table.insert(t,1,{'IF',pred})
    return t
end

function held(t)
    table.insert(t,1,{'HELD'})
    table.insert(t,{'WAIT'})
    table.insert(t,{'UNHELD'})
    return Asl._if( dyn{_held=0}, t)
end

function lock(t)
    table.insert(t,1,{'LOCK'})
    table.insert(t,{'OPEN'})
    return t
end


--- behavioural types
-- available for dynamics so exposed variables can be musician-centric
-- and mutables, where operations are destructive to the value for iteration

local Matheds = {
    step = function(t, inc) return t + inc end,
    mul  = function(t, mul) return t * mul end,
    wrap = function(t, min, max)
        if min == 0 then return t % max
        else return (t - min) % (max - min) + min end
    end,
}

local Mathmt = {
    __unm = function(a)   return Asl.math{'~', a} end,
    __add = function(a,b) return Asl.math{'+', a, b} end,
    __sub = function(a,b) return Asl.math{'-', a, b} end,
    __mul = function(a,b) return Asl.math{'*', a, b} end,
    __div = function(a,b) return Asl.math{'/', a, b} end,
    __mod = function(a,b) return Asl.math{'%', a, b} end, -- % is used to wrap to a range
    __len = function(a)   return Asl.math{'#', a} end, -- freeze operator for mutables
    __index = function(t, ix)
        local fn = Matheds[ix]
        if fn then
            if t[1] == '#' then -- if parent is #(freeze), peel it off
                t = t[2] -- NOTE this doesn't change the self table, must close over
            else -- ==first method ==parent is DYN. convert it to NMUT
                t[2] = {'NMUT', t[2]}
            end
            return function(nop, ...) -- ignore self, instead close over above 't'
                return Asl.math{'#', fn(t, ...)}
            end
        end
    end, -- enable method-chain on dynamics
}
function Asl.math(tab) return setmetatable(tab, Mathmt) end -- overload table with arithmetic semantics

-- usage: dyn{name = default} -- create 'name', initialized to default
-- update: myasl.dyn.name = new_value -- updates the registered 'name' dynamic to value 'new_value'
function dyn(d) return Asl.math{Asl.dyn_compiler, d} end

-- usage: mutable(4) -- creates a literal 4 which can be modified by math ops
-- usage: mutable{ mymut = 3 } -- creates a named mutable so the value can be set directly
-- update named: myasl.mutable.mymut = 42 -- update mymut to value 42
-- NOTE: this type is only useful if math ops are applied
-- ops will be applied on each access to the varable (ie at a breakpoint)
-- use these to build iterators, cycling control flow, algorithmic waveforms
function mutable(n)
    if type(n)=='table' then return dyn({'NMUT', n}) -- named vars are wrapped in dynamics for user control
    else return Asl.math{'MUT', n} end
end


-- composite constructs

function Asl._while(pred, t) return loop( Asl._if(pred, t)) end

function times(n, t) return Asl._while( mutable(n+1)-1, t) end -- n+1 adds before mutation for exactly n repeats


return Asl
