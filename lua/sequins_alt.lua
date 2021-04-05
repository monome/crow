--- sequins
-- nestable tables with sequencing behaviours & control flow
-- TODO i think ASL can be defined in terms of a sequins...


local S = {}

function S.new(t, action)
    -- wrap a table in a sequins with defaults
    local s = { data   = t
              , length = #t -- memoize table length for speed
              , ix     = 1
              }
    s.action = {up = s}
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


function S._step(act)
    local n = resolve(act.n)
    local up = act.up
    local retval, exec = S.next(act)
    if exec ~= 'again' then up.n = up.ix + n end
    if exec == 'dead' then return S.next(act) end -- FIXME add protection for a list of dead sequins (infinite recursion)
    if exec == 'skip' then return S.next(up) end
    return retval
end

function S._every(act)
    -- only return a value every N calls
    local n = resolve(act.n)
    local up = act.up
    local i = act.ix
    if (i % n) == 0 then
        local retval, exec = S.next(act)
        if exec ~= 'again' then act.ix = act.ix + 1 end
        return retval, exec
    end
    act.ix = act.ix + 1
    return {}, 'skip'
end

function S._count(act)
    -- repeat this element N times before releasing control
    local n = resolve(act.n)
    local up = act.up
    local retval, exec = S.next(act)
    if exec == 'skip' then return S.next(up) end -- recur before iterating self.ix
    if exec ~= 'again' then act.ix = act.ix + 1 end
    if act.ix > n then
        act.ix = 1
        return retval, exec 
    end
    return retval, 'again'
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



-- function S.step_RT(self,n) return S.get(self, n, true) end
-- function S.step(n)
--     return function(self) return S.get(self, n, true) end
-- end

-- function S.select(n)
--     return function(self) return S.get(self, n) end
-- end


------------------------------
--- control flow manipulation

function S.next(self)
    local act = self.action
    if not act.fn then
        local s = self.up
        if s.n then -- will be true if a new index needs to be applied
            s.ix = ((s.n - 1) % s.length) + 1 -- wrapping bounds check (note self.n is already resolved)
            s.n = nil -- forget n
        end
        return resolve(s.data[s.ix])
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

function S.select(self, s)return S.extend(self, S._select, s) end
function S.step(self, s)  return S.extend(self, S._step, s) end
function S.every(self, e) return S.extend(self, S._every, e) end
function S.count(self, c) return S.extend(self, S._count, c) end
function S.times(self, n) return S.extend(self, S._times, n) end


--- helpers in terms of core
function S.all(self) return self:count(self.length) end
function S.once(self) return self:times(1) end


-- function S.every(self, e)
--     local function _every(self)
--         local i = self.ix + 1
--         if (i % self.n) == 0 then
--             local retval, exec = self.data()
--             if exec ~= 'again' then self.ix = i end
--             return retval, exec
--         end
--         self.ix = i
--         return 0, 'skip'
--     end
--     return S.wrap(self, _every, e)
-- end

-- function S.count(self, c)
--     local function _count(self)
--         local retval, exec = self.data()
--         if exec == 'skip' then return self:next() end -- recur before iterating self.ix
--         if exec ~= 'again' then self.ix = self.ix + 1 end
--         if self.ix >= self.n then
--             self.ix = 0
--             return retval, exec 
--         end
--         return retval, 'again'
--     end
--     return S.wrap(self, _count, c)
-- end

-- function S.all(self) return self:count(self.length) end

-- function S.times(self, n)
--     local function _times(self)
--         if self.ix < self.n then
--             local retval, exec = self.data()
--             if     exec == 'dead' then -- should propagate up 
--             elseif exec == 'skip' then return self:next() -- recur befor iterating self.ix
--             elseif exec ~= 'again' then self.ix = self.ix + 1
--             end
--             return retval, exec
--         end
--         return 0, 'dead'
--     end
--     return S.wrap(self, _times, n)
-- end


--- metamethods

S.__call = function(self, ...)
    -- print '__call'
    if self == S then -- calling the library itself
        -- TODO check the table reference reliably stays the same?
        -- return S.new(...):step(1)
        return S.new(...):step(1)
    else -- calling default behaviour of an instance
        return S.next(self)
    end
end

S.__index = function(self, ix)
    -- runtime calls to step() and select() should return values, not functions
    -- TODO should these calls update the .next member function?
    if type(ix) == 'number' then return self.data[ix]
    elseif ix == 'next'   then return S.next
    -- elseif ix == 'step'   then return S.step_RT
    elseif ix == 'step'   then return S.step
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
