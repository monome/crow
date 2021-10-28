--- hotswap library

local s = sequins
local tl = timeline

HS = {}

-- to add support for a new type you need: 
-- 1) add an elseif predicate for capturing the type
-- 2) give the type a single character identifier
-- 3) add a swap function to HS._swap table

HS._type = function(o)
    -- return a char representing the type
    if s.is_sequins(o) then return 's'
    elseif tl.is_timeline(o) then return 't'
    else print 'hotswap does not know this type'
    end
end

HS._swap = {
    -- s = s.hotswap -- TODO can just reference the fn directly. not closure
    s = function(old, new) -- sequins updater
        old:settable(new.data)
        --old:hotswap() -- TODO support hotswap natively in sequins
            -- TODO hotswap should handle flow-modifiers & nested sequins
    end,
    t = function(old, new) -- timeline updater
        -- TODO
        -- attempt to handle nested sequins objects
        -- maintain reference beat / time
    end
}

HS._reg = {} -- a place to register updateable sequins
HS.__index = HS._reg

HS.__newindex = function(self, ix, v)
    local t = HS._type(v)
    if t then
        if HS._reg[ix] then -- key already exists
            -- warning! we don't check that the new type matches
            HS._swap[t](HS._reg[ix][2], v)
        else -- new key
            HS._reg[ix] = {t,v} -- register with type annotation
        end
    end
end

return HS
