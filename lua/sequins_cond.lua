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


------------------------------
--- control flow execution

function S.next(self)
    local act = self.action
    if not act.action then -- base case. apply STEP
        local s = act.up

        local newix = s.ix
        local n = resolve(s.n)
        newix = newix + n
        newix = wrap_index(s, newix)
        -- print('new',newix)

        local retval, exec = resolve(s.data[newix])
        -- THIS IS 'STEP'
        if exec ~= 'again' then s.ix = newix end
        -- FIXME add protection for list of dead sequins. for now we just recur, hoping for a live sequin in nest
        -- if exec == 'dead' then return S.next(s) end
        if exec == 'skip' then
            return S.next(s)
        end
        return retval, exec
    else
        return S._generic(act)
    end
end

function S.step(self, s) self.n = s; return self end


------------------------------
--- control flow manipulation

function S._generic(act)
    act.ix = act.ix + 1
    if not act.cond or act.cond(act) then
        retval, exec = S.next(act)
        if exec then act.ix = act.ix - 1 end -- undo increment
    else
        retval, exec = {}, 'skip'
    end
    if act.rcond then
        if act.rcond(act) then
            if exec == 'skip' then retval, exec = S.next(act)
            else exec = 'again' end
        end
    end
    return retval, exec
end

function S.extend(self, t)
    self.action = { up     = self -- containing sequins
                  , action = self.action -- wrap nested actions
                  , ix     = 0
                  }
    for k,v in pairs(t) do self.action[k] = v end
    return self
end

function S._every(self)
    local n = resolve(self.n)
    return (self.ix % n) == 0
end

function S._times(self)
    local n = resolve(self.n)
    return self.ix <= n
end

function S._count(self)
    local n = resolve(self.n)
    if self.ix < n then return true
    else self.ix = 0 end -- reset
end

function S.cond(self, p) return S.extend(self, {cond = p}) end
function S.condr(self, p) return S.extend(self, {cond = p, rcond = p}) end
function S.every(self, n) return S.extend(self, {cond = S._every, n = n}) end
function S.times(self, n) return S.extend(self, {cond = S._times, n = n}) end
function S.count(self, n) return S.extend(self, {rcond = S._count, n = n}) end

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
           , condr    = S.condr
           , every    = S.every
           , times    = S.times
           , count    = S.count
           , all      = S.all
           , once     = S.once
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
