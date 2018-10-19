--- Crow standard library
--

local crow = {}

function crow.squared(n)
    return n*n
end

-- extend this macro to conditionally take 'once' as first arg
--      if present, fn is only executed on first invocation then reused
local function closure_if_table( f )
    local _f = f
    f = function( ... ) -- applies globally
        if type( ... ) == 'table' then
            local args = ...
            return function() return _f( table.unpack(args) ) end
        else
            return _f( ... )
        end
    end
end
-- these functions are overloaded with the table->closure functionality
-- nb: there is a performance penalty to normal usage due to type()
wrapped_fns = { math.random
              , math.min
              , math.max
              }
for _,fn in ipairs( wrapped_fns ) do
    closure_if_table( fn )
end

print'crow lib loaded'

return crow
