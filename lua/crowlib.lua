--- Crow standard library

local crow = {}

crow.version = '0.0.0'

function whatversion()
    return crow.version
end

function printversion()
    print( 'crow ' .. whatversion() )
end

--- Library loader
--
local function closelibs()
    -- set whole list of libs to nil to close them
    -- TODO does this free the RAM used by 'dofile'?
    Asl    = nil
    Asllib = nil
    Metro  = nil
end

function crow.libs( lib )
    if lib == nil then
        -- load all
        Asl    = dofile('lua/asl.lua')
        Asllib = dofile('lua/asllib.lua')
        Metro  = dofile('lua/metro.lua')
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


--- Output lib
-- 4 outputs
out = {1,2,3,4}

--- Asl
function toward_handler( id ) end -- do nothing if Asl not active
-- if defined, make sure active before setting up actions and banging
if Asl then
    toward_handler = function( id )
        out[id].asl:step()
    end
end
-- TODO should 'go_toward' be called 'slew'???
-- special wrapper should really be in the ASL lib itself?
function LL_toward( id, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    if type(s) == 'function' then s = s() end
    --print('id: '..id,'\ttoward '..d,'\tin time: '..t,'\twith shape: '..s)
    go_toward( id, d, t, s )
end

function LL_get_state( id )
    return get_state(id)
end



-- MUST setup Asl before applying actions
for chan = 1, #out do
    -- pass the result of out.new() ?
    out[chan] = { channel = chan
                , level   = 1.0
                , rate    = 1.0
                , shape   = 'linear'
                , asl     = Asl.new(chan) -- the standard LFO
--                , trig    = { asl      = Asl.new(k)
--                            , polarity = 1
--                            , time     = 1
--                            , level    = 5
--                            }
                }
    out[chan].asl.action = lfo( function() return out[chan].rate  end
                              , function() return out[chan].shape end
                              , function() return out[chan].level end
                              )
    --out[chan].asl:action( toward( 1, 10, 'linear' ) )
--    out[chan].trig.asl:action( trig( function() return out[chan].trig.polarity end
--                                , function() return out[chan].trig.time end
--                                , function() return out[chan].trig.level end
--                                )
--                          )
    -- consider end of trig causing 'bang' of action if it exists?
--    out[chan].asl:bang(true)
end

adc_remote = function(chan)
    get_cv(chan)
end



--- Communication functions
-- these will be called from norns (or the REPL)
-- they return values wrapped in strings that can be used in Lua directly
-- via dostring
get_cv_cb = 'ret_cv' -- make a list of these so they can be queried / changed
get_out_cb = 'out_cv' -- make a list of these so they can be queried / changed
function get_cv( channel )
    print('^^' .. get_cv_cb .. '(' .. channel .. ',' .. io_get_input(channel) .. ')')
end
function get_out( channel )
    print('^^' .. get_out_cb .. '(' .. channel .. ',' .. get_state(channel) .. ')')
end

-- do we always have fixed arg count? do all args always get send to c-fn?
remotes = { { 'get_cv', 'ret_cv', io_get_input }
          , { 'get_out', 'out_cv', get_state   }
          }

--function make_remote_fns()
--    --TODO
--end
--
--make_remote_fn
--    function get_out( channel )
--        print('^^' .. cb .. '(' .. channel .. ',' .. get_state(channel) .. ')')
--    end

--- Flash program
function start_flash_chunk()
    -- should kill the currently running lua script
    -- turn off timers & callbacks? else?
    -- call to C to switch from REPL to writer
end





--- Syntax extensions
-- 
-- extend this macro to conditionally take 'once' as first arg
--      if present, fn is only executed on first invocation then reused
function closure_if_table( f )
    local _f = f
    f = function( ... ) -- applies globally
        if type( ... ) == 'table' then
            local args = ...
            return function() return _f( table.unpack(args) ) end
        else
            return _f( ... )
        end
    end
    return f
end
-- these functions are overloaded with the table->closure functionality
-- nb: there is a performance penalty to normal usage due to type()
wrapped_fns = { math.random
              , math.min
              , math.max
              }
for _,fn in ipairs( wrapped_fns ) do
    fn = closure_if_table( fn )
end

print'crowlib loaded'

return crow
