--- sequins
-- nestable tables with sequencing behaviours & control flow
-- TODO i think ASL can be defined in terms of a sequins...


local S = {}

function S.wrap(t, next, n)
    -- wrap a table in a sequins with defaults
    local s = { data   = t
              , length = t.length or #t -- borrow child length or memoize raw table length for speed
              , next   = next or S.step(1)
              , ix     = 1
              , n      = n -- n can be a sequins. a value will be resolved per call
              }
    setmetatable(s, S)
    return s
end

-- can this be generalized to cover every/count/times etc
function S.setdata(self, t)
    
    self.data   = t
    self.length = #t
end



------------------------------
--- behaviours

local function resolve(t)
    if getmetatable(t) == S then return t:next() end
    return t
end

---- select is special bc it is the *onlY* behaviour of a naked sequins
-- the only way to change what a sequins returns is by running an s.ix transformer fn
-- self.n is where a new index is 'loaded', then _select bounds-checks it and loads into self.ix
function S._select(self)
    if self.n then -- will be true if a new index needs to be applied
        self.ix = ((self.n - 1) % self.length) + 1 -- wrapping bounds check (note self.n is already resolved)
        self.n = nil -- forget n
    end
    return resolve(self.data[self.ix])
end

function S._step(self)
    local n = resolve(self.n)
    local retval, exec = self.data:next()
    if exec ~= 'again' then self.data.n = self.data.ix + n end
    if exec == 'dead' then return self:next() end -- FIXME add protection for a list of dead sequins (infinite recursion)
    if exec == 'skip' then return self:next() end
    return retval
end

function S._every(self)
    local n = resolve(self.n)
    local i = self.ix + 1
    if (i % n) == 0 then
        local retval, exec = self.data:next()
        if exec ~= 'again' then self.ix = i end
        return retval, exec
    end
    self.ix = i
    return {}, 'skip'
end

function S._count(self)
    local n = resolve(self.n)
    local retval, exec = self.data:next()
    if exec == 'skip' then return self:next() end -- recur before iterating self.ix
    if exec ~= 'again' then self.ix = self.ix + 1 end
    if self.ix >= n then
        self.ix = 0
        return retval, exec 
    end
    return retval, 'again'
end

function S._times(self)
    local n = resolve(self.n)
    if self.ix >= n then
        return {}, 'dead'
    else
        local retval, exec = self.data:next()
        if     exec == 'dead' then -- should propagate up 
        elseif exec == 'skip' then return self:next() -- recur befor iterating self.ix
        elseif exec ~= 'again' then self.ix = self.ix + 1
        end
        return retval, exec
    end
end



function S.step_RT(self,n) return S.get(self, n, true) end
function S.step(n)
    return function(self) return S.get(self, n, true) end
end

function S.select(n)
    return function(self) return S.get(self, n) end
end


------------------------------
--- control flow manipulation

-- function S.step(self, s)  return S.wrap(self, S._step, s) end
-- function S.select(self, s)return S.wrap(self, S._select, s) end
-- function S.every(self, e) return S.wrap(self, S._every, e) end
-- function S.count(self, c) return S.wrap(self, S._count, c) end
-- function S.times(self, n) return S.wrap(self, S._times, n) end


-- --- helpers in terms of core
-- function S.all(self) return self:count(self.length) end
-- function S.once(self) return self:times(1) end


function S.every(self, e)
    local function _every(self)
        local i = self.ix + 1
        if (i % self.n) == 0 then
            local retval, exec = self.data()
            if exec ~= 'again' then self.ix = i end
            return retval, exec
        end
        self.ix = i
        return 0, 'skip'
    end
    return S.wrap(self, _every, e)
end

function S.count(self, c)
    local function _count(self)
        local retval, exec = self.data()
        if exec == 'skip' then return self:next() end -- recur before iterating self.ix
        if exec ~= 'again' then self.ix = self.ix + 1 end
        if self.ix >= self.n then
            self.ix = 0
            return retval, exec 
        end
        return retval, 'again'
    end
    return S.wrap(self, _count, c)
end

function S.all(self) return self:count(self.length) end

function S.times(self, n)
    local function _times(self)
        if self.ix < self.n then
            local retval, exec = self.data()
            if     exec == 'dead' then -- should propagate up 
            elseif exec == 'skip' then return self:next() -- recur befor iterating self.ix
            elseif exec ~= 'again' then self.ix = self.ix + 1
            end
            return retval, exec
        end
        return 0, 'dead'
    end
    return S.wrap(self, _times, n)
end


--- metamethods

S.__call = function(self, ...)
    -- print '__call'
    if self == S then -- calling the library itself
        -- TODO check the table reference reliably stays the same?
        return S.wrap(...)--:step(1)
    else -- calling default behaviour of an instance
        return self.next(self) -- do we want to handle arguments?
    end
end

S.__index = function(self, ix)
    -- runtime calls to step() and select() should return values, not functions
    -- TODO should these calls update the .next member function?
    if type(ix) == 'number' then return self.data[ix]
    elseif ix == 'step'   then return S.step1_RT
    -- elseif ix == 'step'   then return S.step
    elseif ix == 'select' then return S.get
    elseif ix == 'settable' then return S.setdata
    elseif ix == 'every' then return S.every
    elseif ix == 'count' then return S.count
    elseif ix == 'times' then return S.times
    elseif ix == 'all' then return S.all
    end
end

S.__newindex = function(self, ix, v)
    if type(ix) == 'number' then self.data[ix] = v
    elseif ix == 'n' then rawset(self,ix,v)
    end
end


setmetatable(S, S)

return S
