local Asl = {}

local Dynmt = {
    __newindex = function(self, k, v) casl_setdynamic(self.id, self._names[k], v) end,
    __index = function(self, k) return casl_getdynamic(self.id, self._names[k]) end
}

function Asl.new(id)
    local c = {id = id or 1}
    c.dyn = setmetatable({_names={}, id=c.id}, Dynmt) -- needs link to `id`
    c.iter = c.dyn -- alias iter to dyn for more natural manipulation of named iterables
    setmetatable(c, Asl)
    return c
end

function Asl.link(self, tab)
    for k,v in pairs(tab) do
        local typ = type(v)
        if typ == 'function' then
            return v(self, tab[2]) -- call compiler fn with self & return new table
            -- early return bc fn must be in the first position, and consumes following arg
        elseif typ == 'table' then
            tab[k] = Asl.link(self, v) -- link nested tables (won't copy unchanged tables)
        end
    end
    return tab
end

function Asl:describe(d)
    casl_describe(self.id, Asl.link(self, d))
end

function Asl.set_held(self, b)
    if self.dyn._held then
        self.dyn._held = b
        return true -- ie. success in setting
    end
end

-- direc(tive) can take 0/1 of false/true. ONLY has effect if there is a held{} construct
-- truthy always restarts 
-- falsey means 'release'
function Asl:action(direc)
    if not direc then -- no arg is always 'restart'
        casl_action(self.id, 1)
    elseif direc == 'unlock' then casl_action(self.id, 2) -- release lock construct
    else -- set `held` dyn if it exists. call action unless no `held` and direc is falsey
        local s = (direc == true or direc == 1) and 1 or 0
        if Asl.set_held(self, s) or s==1 then casl_action(self.id, s) end
    end
end

function Asl._if(pred, tab)
    table.insert(tab,1,{'IF',pred})
    return tab
end

function Asl.dyn_compiler(self, d)
    -- register a dynamic pair {name=default}, and return a reference to it
    local ref = 0
    for k,v in pairs(d) do -- ONLY ONE ELEMENT!
        if not self.dyn._names[k] then -- check if it's a new entry
            self.dyn._names[k] = casl_defdynamic(self.id)
        end
        ref = self.dyn._names[k]
        self.dyn[k] = v -- set the default
        break -- only a single iteration
    end
    return {'DYN', ref}
end

function Asl.iter_compiler(self, n)
    if type(n) == 'table' then
        print 'named iterable'
        -- TODO named counter: use a dynamic to allow updating
    end
    return {'ITER', n}
end


-- metatables
Asl.__index = function(self, ix)
    if     ix == 'describe' then return Asl.describe
    elseif ix == 'action' then return Asl.action
    end
end
setmetatable(Asl, Asl)


-- global functions
function to(volts, time, shape)
    return {'TO', volts or 0.0, time or 1.0, shape or 'linear'}
end

function loop(t)
    table.insert(t,{'RECUR'}) -- TODO does this need to be wrapped in a table?
    return t
end

function dyn(d)
    return {Asl.dyn_compiler, d} -- defer to allow dyn to be captured per instance
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

function Asl._while(pred, t)
    t = Asl._if(pred, t)
    return loop(t)
end

--- iterable tables can have the standard math operators applied to them
-- each operation wraps the table in another iterable table
local Itermt = {
    __unm = function(a)   return Asl._iter{'~', a} end,
    __add = function(a,b) return Asl._iter{'+', a, b} end,
    __sub = function(a,b) return Asl._iter{'-', a, b} end,
    __mul = function(a,b) return Asl._iter{'*', a, b} end,
    __div = function(a,b) return Asl._iter{'/', a, b} end,
    __mod = function(a,b) return Asl._iter{'%', a, b} end, -- % is used to wrap to a range
}

function Asl._iter(tab)
    return setmetatable(tab, Itermt)
end

function iterable(n)
    -- if the iterable is named, create a dynamic to allow updating
    if type(n)=='table' then return Asl._iter(dyn(n)) -- tables are wrapped into dynamics
    else return Asl._iter{n} end
end

function times(n, t)
    return Asl._while( iterable(n)-1, t)
end


return Asl
