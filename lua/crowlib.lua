--- Crow standard library

local crow = {}


--- Library loader
--
local function closelibs()
    -- set whole list of libs to nil to close them
    -- TODO does this free the RAM used by 'dofile'?
    Asl = nil
end

function crow.libs( lib )
    if lib == nil then
        -- load all
        Asl = dofile('lua/asl.lua')
    elseif type(lib) == 'table' then
        -- load the list 
    else
        if lib == 'close' then closelibs() end
        -- assume string & load single library
    end
end

-- TODO open all libs by default
crow.libs()


--- Utilities
--
-- just demonstrating how to make crow.* fns
function crow.squared(n)
    return n*n
end



--- Hardware I/O
--



--- Output lib
-- 4 outputs
local out = {1,2,3,4}
for k in ipairs( out ) do
    -- pass the result of out.new() ?
    out[k] = { channel = k
             , level   = 0
             , time    = 0
             , slope   = 'linear'
             }
end

-- TODO should 'go_toward' be called 'slew'???


--- ASL specifics
-- actually maybe there should be some further breakdown
-- in/out/timers/ii get their own files
-- crowlib provides only the lowest level access fns
-- though this is still relatively high-level vs C

-- need one of these per 'out' class
local slope = {}
-- handles callback from C-dsp engine
-- this should do nothing by default, then the asl-out-extension
-- should overwrite it to the following.
-- then the user can define their own callback handler without
-- the full asl library if they want
function toward_handler( id )
    slope[id]:step()
end
-- special wrapper should really be in the ASL lib itself?
function LL_toward( id, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    go_toward( id, d, t, s )
end



--- Syntax extensions
-- 
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
