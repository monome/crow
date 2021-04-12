--- sequins
-- nestable tables with sequencing behaviours & control flow
-- TODO i think ASL can be defined in terms of a sequins...


local S = {}

function S.new(t)
    -- wrap a table in a sequins with defaults
    local s = { data   = t
              , length = #t -- memoize table length for speed
              , ix     = #t
              , n      = 1 -- can be a sequin or a function
              }
    s.action = {up = s}
    setmetatable(s, S)
    return s
end

local function wrap_index(s, ix)
    return ((ix - 1) % s.length) + 1
end

-- can this be generalized to cover every/count/times etc
function S.setdata(self, t)
    self.data   = t
    self.length = #t
    self.ix = wrap_index(self, self.ix)
end



------------------------------
--- behaviours

local function resolve(t)
    -- this is the link between nested sequins
    if getmetatable(t) == S then return S.next(t) end
    return t
end

function S._every(act)
    -- only return a value every N calls
    local n = resolve(act.n)
    local up = act.up
    local i = act.ix
    local retval, exec = {}, 'skip'
    if (i % n) == 0 then
        retval, exec = S.next(act)
    end
    if exec ~= 'again' then act.ix = act.ix + 1 end
    return retval, exec
end

function S._count(act)
    -- repeat this element N times before releasing control
    local n = resolve(act.n)
    local up = act.up
    local retval, exec = S.next(act)
    if exec == 'skip' then return S.next(up) -- recur before iterating self.ix
    elseif exec ~= 'again' then act.ix = act.ix + 1 end
    if act.ix <= n then
        exec = 'again'
    else
        act.ix = 1
    end
    return retval, exec 
end

function S._times(act)
    -- return the N values before returning nothing forever
    local n = resolve(act.n)
    local up = act.up
    if act.ix > n then
        return {}, 'dead'
    else
        local retval, exec = S.next(act)
        if     exec == 'dead' then -- should propagate up 
        elseif exec == 'skip' then return S.next(up) -- recur befor iterating self.ix
        elseif exec ~= 'again' then act.ix = act.ix + 1
        end
        return retval, exec
    end
end

function S._cond(act)
    -- checks predicate, and returns a value if true, else returns 'skip'
    local n = resolve(act.n)
    local up = act.up
    local retval, exec = {}, 'skip'
    if n() == 1 then -- does n() need to be passed self?
        retval, exec = S.next(act)
    end
    return retval, exec
end


------------------------------
--- control flow manipulation

function S.next(self)
    local act = self.action
    if not act.fn then -- base case. apply STEP
        local s = act.up

        local newix = s.ix
        local n = resolve(s.n)
        newix = newix + n
        newix = wrap_index(s, newix)

        local retval, exec = resolve(s.data[newix])
        -- THIS IS 'STEP'
        if exec ~= 'again' then s.ix = newix end
        -- FIXME add protection for list of dead sequins. for now we just recur, hoping for a live sequin in nest
        if exec == 'dead' then return S.next(s) end
        if exec == 'skip' then return S.next(s) end

        return retval -- NOTE remove exec as 'again' doesn't pass up the chain
    else
        return act.fn(act)
    end
end

function S.extend(self, fn, n)
    self.action = { up     = self -- containing sequins
                  , action = self.action -- wrap nested actions
                  , fn     = fn
                  , ix     = 1
                  , n      = n
                  }
    return self
end

function S.step(self, s) self.n = s; return self end

function S.cond(self, pred) return S.extend(self, S._cond, pred) end
function S.every(self, e) return S.extend(self, S._every, e) end
function S.count(self, c) return S.extend(self, S._count, c) end
function S.times(self, n) return S.extend(self, S._times, n) end

--- helpers in terms of core
function S.all(self) return self:count(self.length) end
function S.once(self) return self:times(1) end


--- metamethods

S.__call = function(self, ...)
    if self == S then -- calling the library itself
        return S.new(...)
    else -- calling default behaviour of an instance
        return S.next(self)
    end
end

S.metaix = { settable = S.setdata
           , step     = S.step
           , cond     = S.cond
           , every    = S.every
           , count    = S.count
           , all      = S.all
           , times    = S.times
           }
S.__index = function(self, ix)
    -- runtime calls to step() and select() should return values, not functions
    if type(ix) == 'number' then return self.data[ix]
    else
        return S.metaix[ix]
    end
end

S.__newindex = function(self, ix, v)
    if type(ix) == 'number' then self.data[ix] = v
    elseif ix == 'n' then rawset(self,ix,v)
    end
end


setmetatable(S, S)

return S