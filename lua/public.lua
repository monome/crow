--- public library
local Public = {
    _names = {}, -- keys are names, linked to indexed _params
    _params = {}, -- storage of params
    view = {
    	input = {},
    	output = {},
    },
}

for n=1,2 do Public.view.input[n] = function(b)
		if b==nil then b = 1 end -- no arg enables
		pub_view_in(n, b)
	end
end
for n=1,4 do Public.view.output[n] = function(b)
		if b==nil then b = 1 end -- no arg enables
		pub_view_out(n, b)
	end
end

Public.view.all = function(b)
	for n=1,2 do Public.view.input[n](b) end
	for n=1,4 do Public.view.output[n](b) end
end

-- Pubtable for indexing methods
local Pubtable = {} -- metamethods for methods on a tables in the public variable space

Pubtable.new = function(name, p) -- converts an array-table into a pubtable
	-- p.v doesn't exist. accessed through metamethods
	p.k = name
	p._index = 0 -- invalid index
	p._step = 1 -- default step motion
	setmetatable(p, Pubtable) -- overload the parameter with table semantics
	return p
end

local function indexwrap(p, ix)
	-- don't use # op because it can include old table entries
	local len = 0; for _,_ in ipairs(p) do len = len + 1 end
	return ((ix-1) % len) + 1 -- wrap to table size
end

Pubtable.__newindex = function(self, ix, val)
	if ix == 'v' then
		for i=1,#val do self[i] = val[i] end -- copy over old table
		self[#val+1] = nil -- mark end of table so ipairs ignores older longer lists
		self._index = indexwrap(self, self._index)
	elseif ix == 'index' then -- like select(n) but no broadcast
		self._index = indexwrap(self, val)
	else
		rawset(self, ix, val)
	end
end

Pubtable.__index = function(self, ix) -- getters
	if ix == 'select' then
		return function(n)
			self._index = indexwrap(self, n or self._index) -- no arg, re-indexes (use after changing table structure)
			Public.broadcast(self.k, self._index, 'index')
			return self[self._index]
		  end
	elseif ix == 'step' then
		return function(n)
			self._step = n or self._step -- no arg uses last step
			self._index = indexwrap(self, self._index + self._step)
			Public.broadcast(self.k, self._index, 'index')
			return self[self._index]
		  end
	elseif ix == 'v' then
		return self
	elseif ix == 'index' then
		return self._index
	end
end

setmetatable(Pubtable, Pubtable)

Public.Pubtable = Pubtable -- capture into public lib

-- get the value of a named public parameter
Public.unwrap = function(name) return Public._params[ Public._names[name] ] end

Public.add = function(name, default, typ, action)
  -- link index & name
	local ix = Public._names[name]
	if not ix then
		ix = #(Public._params) + 1
		Public._names[name] = ix
	end

  -- initialze name & value
	if type(default) == 'table' then
		Public._params[ix] = Public.Pubtable.new(name, default)
	else
		Public._params[ix] = { k=name, v=default or 0 }
	end

  -- register type metadata
	if typ then
		local p = Public._params[ix] -- alias
		local t = type(typ)
	  -- register action
		if action then p.action = action end
		if t == 'function' then p.action = typ
	  -- string in typ position means literal with special external handling
		elseif t == 'string' then p.type = typ
	  -- table of metadata
		else
			if type(typ[1]) == 'string' then
				p.type = 'option'
				p.option = typ
				p.noitpo = {} -- build a reverse-lookup table
				for i=1,#typ do p.noitpo[typ[i]] = i end
			else
				p.min = typ[1]
				p.max = typ[2]
				p.type = typ[3]
			end
		end
	end
end

Public.clear = function()
	Public._names = {}
	Public._params = {}
end

local function quoteptab(p, index)
	local t = {}
	for i=1,#p.v do
		if type(p.v[i] == 'string') then t[i] = quotes(p.v[i])
        else t[i] = p.v[i] end
	end
	if index then t[#t+1] = string.format('index=%g',p._index) end
	return string.format('{%s}', table.concat(t,','))
end

local function dval(p)
	local tv = type(p.v)
	if tv == 'string' then
		return quotes(p.v)
	elseif tv == 'table' then
		return quoteptab(p, true)
    else
        return p.v
	end
end

local function dtype(p)
	local t = {}
	if p.option then
		t = {string.format('%q', 'option')}
		for i=1,#p.option do
			t[#t+1] = quotes(p.option[i])
		end
	else
		-- TODO update to use quote() instead of string.format
		if p.min then table.insert(t, string.format('%g', p.min)) end
		if p.max then table.insert(t, string.format('%g', p.max)) end
		if p.type then table.insert(t, string.format('%q', p.type)) end
	end
	return table.concat(t,',')
end

Public.discover = function()
	for _,p in ipairs(Public._params) do
        _c.tell('pub', quotes(p.k), dval(p), string.format('{%s}',dtype(p)))
	end
	_c.tell('pub',quotes'_end')
end

Public.doact = function(p, v)
	if p.action then p.action(v) end
end

local function clampn(v, min, max)
	return (v < min) and min
		or (v > max) and max
        or v
end

Public.clamp = function(p, val)
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

Public.broadcast = function(k, v, kk)
	local tv = type(v)
	if tv == 'string' then v = quotes(v)
	elseif tv == 'table' then v = quoteptab(v, true) end
    -- else v = v
	if kk then
		_c.tell('pupdate', quotes(k), v, quotes(kk))
	else _c.tell('pupdate', quotes(k), v) end
end

-- NOTE: To only be called by remote device
-- TODO add optional 3rd arg for updating a table member
Public.update = function(k,v,kk)
	local p = Public.unwrap(k)
	if p then
		if kk then -- setting a table-element
			if type(kk) == 'number' then v = Public.clamp(p, v) end
			p[kk] = v
		else
			p.v = Public.clamp(p, v)
		end
		Public.doact(p, p.v)
	end
end

--- METAMETHODS
-- self.ix = val
Public.__newindex = function(self, ix, val)
	-- TODO identical to .update but without .broadcast. unify after adding table support
	local p = Public.unwrap(ix)
	if p then
		p.v = Public.clamp(p, val)
		Public.broadcast(p.k, p.v)
		Public.doact(p, p.v)
	end
end

-- self.ix._ or val = self.ix
Public.__index = function(self, ix)
	local p = Public.unwrap(ix)
	if p then return p.v end
end

setmetatable(Public, Public)

return Public
