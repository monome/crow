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
    table.insert(t,{'RECUR'})
    return t
end

function dyn(d)
    return {Asl.dyn_compiler, d} -- defer to allow dyn to be captured per instance
end

function held(t)
    return Asl._if( dyn{_held=0}, t)
end



return Asl
