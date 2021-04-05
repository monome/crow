--- sequins
-- nestable tables with sequencing behaviours & control flow
-- TODO i think ASL can be defined in terms of a sequins...


local S = {}

function S.new(t, behaviour)
    -- overload table with Sequins behaviour
    t.length = #t -- memoize length for speed
    t.next = behaviour or S.step(1)
    t.index = 0 -- default behaviour will thus start at 1
    setmetatable(t, S)
end


--- behaviours

function S.resolve(self)
    -- compute recursive sequins
    if getmetatable(self) == S then
        print 'resolving'
        return self.next(self) -- resolve by inquiring for a value
    end
    return self
end

function S.get(self, n, adder)
    -- general bounds-checked index update & return
    -- n = S.resolve(n) -- handle an n which is a sequins
    if adder then n = n + adder else
    self.index = ((n-1) % self.length) + 1 -- wrap
    return S.resolve(self[self.index]) -- recursively resolve until a real value is returned
end

function S.step(n)
    return function(self) S.get(self, n, self.index) end
end

function S.select(n)
    return function(self) S.get(self, n) end
end


--- control flow manipulation

function S.every(self, e)
    -- TODO
    return self
end

function S.count(self, c)
    -- TODO
    return self
end

function S.all(self)
    -- TODO
    return self
end


--- metamethods

S.__call = function(self, ...)
    print '__call'
    if self == S then -- calling the library itself
        -- TODO check the table reference reliably stays the same?
        return S.new(...)
    else -- calling default behaviour of an instance
        return self.next(self) -- do we want to handle arguments?
    end
end

S.__index = function(self, ix)
    -- runtime calls to step() and select() should return values, not functions
    -- TODO should these calls update the .next member function?
    if     ix == 'step'   then return function(self,n) S.get(self,n,self.index) end
    elseif ix == 'select' then return S.get
    end
end


setmetatable(S, S)

return S
