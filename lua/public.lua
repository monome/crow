--- public library
local P = {
    _names = {}, -- keys are names, linked to indexed _params
    _params = {}, -- storage of params
    view = {
        input = {},
        output = {},
    },
}

for n=1,2 do P.view.input[n] = function(b)
        if b==nil then b = 1 end -- no arg enables
        pub_view_in(n, b)
    end
end
for n=1,4 do P.view.output[n] = function(b)
        if b==nil then b = 1 end -- no arg enables
        pub_view_out(n, b)
    end
end

P.view.all = function(b)
    for n=1,2 do P.view.input[n](b) end
    for n=1,4 do P.view.output[n](b) end
end


-- get the value of a named public parameter
P.unwrap = function(name) return P._params[ P._names[name] ] end


P._chain = {
    -- all fns must return p for method chaining
    __index = { range   = function(p,m,x) p.min, p.max = m, x; return p end
              , xrange  = function(p,m,x) p.min, p.max, p.tipe = m, x, 'exp'; return p end
              , options = function(p,os)
                    p.tipe, p.option, p.noitpo = 'option', os, {}
                    for i=1,#os do p.noitpo[os[i]] = i end -- reverse-lookup
                    return p
                end
              , type    = function(p,t) p.tipe = t; return p end
              , action  = function(p,f) p.act = f; return p end
              }
}

P.add = function(name, default, typ, action)
    if type(name) == 'table' then -- assume method-chain style
        k,v = next(name) -- table should have a single k,v pair entry
        default = v
        name = k
    end
  -- link index & name
    local ix = P._names[name]
    if not ix then
        ix = #(P._params) + 1
        P._names[name] = ix
    end
  -- initialze name & value
    P._params[ix] = { k=name, v=default or 0 }
    local p = P._params[ix] -- alias
    if sequins.is_sequins(p.v) then
        p.sequins = true
        p._index = p.v.ix
    end
  -- register type metadata
    if typ then
        local t = type(typ)
      -- register action
        if action then p.act = action end
        if t == 'function' then p.act = typ
      -- string in typ position means literal with special external handling
        elseif t == 'string' then p.tipe = typ
      -- table of metadata
        else
            if type(typ[1]) == 'string' then
                p.tipe = 'option'
                p.option = typ
                p.noitpo = {} -- build a reverse-lookup table
                for i=1,#typ do p.noitpo[typ[i]] = i end
            else
                p.min = typ[1]
                p.max = typ[2]
                p.tipe = typ[3]
            end
        end
    end
    return setmetatable(p, P._chain) -- return a reference to the new public table entry
end

P.clear = function()
    P._names = {}
    P._params = {}
    P.view.all(false)
    _c.tell('pub',quote'_clear')
end

local function quoteptab(p)
    -- TODO optimize this to leverage quote library
    local t = {}
    local i = 1 -- manual iteration to enable table or sequins (ipairs won't work with sequins)
    while p.v[i] ~= nil do
        if type(p.v[i] == 'string') then t[i] = quote(p.v[i])
        else t[i] = p.v[i] end
        i = i + 1
    end
    if p.sequins then t[i] = string.format('index=%g',p.v.ix) end
    return string.format('{%s}', table.concat(t,','))
end

local function dval(p)
    local tv = type(p.v)
    if tv == 'string' then return quote(p.v)
    elseif tv == 'table' then return quoteptab(p)
    else return p.v
    end
end

local function dtype(p)
    local t = {}
    if p.option then
        t = {string.format('%q', 'option')}
        for i=1,#p.option do
            t[#t+1] = quote(p.option[i])
        end
    else
        if p.min then table.insert(t, quote(p.min)) end
        if p.max then table.insert(t, quote(p.max)) end
        if p.tipe then table.insert(t, quote(p.tipe)) end
    end
    return table.concat(t,',')
end

P.discover = function()
    for _,p in ipairs(P._params) do
        _c.tell('pub', quote(p.k), dval(p), string.format('{%s}',dtype(p)))
    end
    _c.tell('pub',quote'_end')
end

P.doact = function(p, v)
    if p.act then p.act(v) end
end

local function clampn(v, min, max)
    return (v < min) and min
        or (v > max) and max
        or v
end

P.clamp = function(p, val)
    if p.min then
        if type(val) == 'table' then
            for k,v in ipairs(val) do
                val[k] = clampn(v, p.min, p.max)
            end
        else
            val = clampn(val, p.min, p.max)
        end
    elseif p.option then
        if not p.noitpo[val] then val = p.v end -- disallow changes out of option
    end
    return val
end

P.broadcast = function(k, v, kk)
    local tv = type(v)
    if tv == 'string' then v = quote(v)
    elseif tv == 'table' then v = quoteptab(v) end
    -- else v = v
    if kk then
        _c.tell('pupdate', quote(k), v, quote(kk))
    else _c.tell('pupdate', quote(k), v) end
end

-- NOTE: To only be called by remote device
P.update = function(k,v,kk)
    local p = P.unwrap(k)
    if p then
        if kk then -- setting a table / sequins element
            if type(kk) == 'number' then v = P.clamp(p, v) end -- setting a data element
            p.v[kk] = v
        elseif sequins.is_sequins(p.v) then -- sequins type
            p.v:settable(P.clamp(p, v))
        else -- number, option, table types
            p.v = P.clamp(p, v)
        end
        P.doact(p, p.v)
    end
end

P.did_update = function(p, last)
    -- check if anything changed in sequins that should be sent
    if last ~= p.v.ix then -- index was changed
        P.broadcast(p.k, p.v.ix, 'index')
    -- else
        -- TODO how do we detect what changed in the sequins? just send it all?
    end
end

--- METAMETHODS
-- self.ix = val
P.__newindex = function(self, ix, val)
    -- TODO identical to .update but adds .broadcast. unify after adding table support
    local p = P.unwrap(ix)
    if p then
        p.v = P.clamp(p, val)
        P.broadcast(p.k, p.v)
        P.doact(p, p.v)
    end
end

-- self.ix._ or val = self.ix
P.__index = function(self, ix)
    local p = P.unwrap(ix)
    if p then
        if p.sequins then -- defer query of whether something changed to main loop
            clock.run(function() local i=p.v.ix; clock.sleep(0); P.did_update(p, i) end)
        end
        return p.v
    end
end

-- call public itself as a shortcut to public.add
P.__call = function(self, ...) return P.add(...) end

setmetatable(P, P)

return P
