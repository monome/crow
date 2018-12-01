--- Crow standard library

local crow = {}


--- Library loader
--
local function closelibs()
    -- set whole list of libs to nil to close them
    -- TODO does this free the RAM used by 'dofile'?
    Asl    = nil
    Asllib = nil
end

function crow.libs( lib )
    if lib == nil then
        -- load all
        Asl    = dofile('lua/asl.lua')
        Asllib = dofile('lua/asllib.lua')
    elseif type(lib) == 'table' then
        -- load the list 
    else
        if lib == 'close' then closelibs() end
        -- assume string & load single library
    end
end

-- open all libs by default
crow.libs()



--- Hardware I/O
--



--- Output lib
-- 4 outputs
out = {1,2,3,4}
for k in ipairs( out ) do
    -- pass the result of out.new() ?
    out[k] = { channel = k
             , level   = 0
             , rate    = 0
             , shape   = 'linear'
             , asl     = Asl.new(k) -- the standard LFO
             , trig    = { asl      = Asl.new(k)
                         , polarity = 1
                         , time     = 1
                         , level    = 5
                         }
             }
    out[k].asl:action( lfo( function() return out[k].rate end
                          , function() return out[k].shape end
                          , function() return out[k].level end
                          )
                     )
    out[k].trig.asl:action( trig( function() return out[k].trig.polarity end
                                , function() return out[k].trig.time end
                                , function() return out[k].trig.level end
                                )
                          )
    -- consider end of trig causing 'bang' of action if it exists?
end

-- Asl redefines this to provide actions at end of slopes
function toward_handler( id ) end


--- Asl
-- get a new Asl object for each out channel
-- redefine the toward_handler
--if Asl then
--    for k in ipairs( out ) do
--        out[k].asl = Asl.new(k)
--    end
--    toward_handler = function( id )
--        out[id].asl:step()
--    end
--end



-- TODO should 'go_toward' be called 'slew'???
-- special wrapper should really be in the ASL lib itself?
function LL_toward( id, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    go_toward( id, d, t, s )
end

function LL_get_state( id )
    return get_state(id)
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

print'crowlib loaded'

return crow
