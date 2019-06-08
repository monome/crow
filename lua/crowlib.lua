--- Crow standard library

local _crow = {}
local _c = _crow -- alias


--- Library loader
--
local function closelibs()
    -- set whole list of libs to nil to close them
    -- TODO does this free the RAM used by 'dofile'?
    Input  = nil
    Output = nil
    Asl    = nil
    Asllib = nil
    Metro  = nil
    metro  = nil --alias
    II     = nil
    Cal    = nil
    Midi   = nil
end

function _crow.libs( lib )
    if lib == nil then
        -- load all
        Input  = dofile('lua/input.lua')
        Output = dofile('lua/output.lua')
        Asl    = dofile('lua/asl.lua')
        Asllib = dofile('lua/asllib.lua')
        Metro  = dofile('lua/metro.lua')
        metro  = Metro --alias
        II     = dofile('lua/ii.lua')
        Cal    = dofile('lua/calibrate.lua')
        Midi   = dofile('lua/midi.lua')
    elseif type(lib) == 'table' then
        -- load the list 
    else
        if lib == 'close' then closelibs() end
        -- assume string & load single library
    end
end

-- open all libs by default
_crow.libs()

--- Communication functions
-- these will be called from norns (or the REPL)
-- they return values wrapped in strings that can be used in Lua directly
-- via dostring

--TODO tell should be in c-fns table, not _crow table?
function _crow.tell( event_name, ... )
    tell( event_name, ... )
end

function get_out( channel )
    _c.tell( 'out_cv', channel, get_state( channel ))
end
function get_cv( channel )
    _c.tell( 'ret_cv', channel, io_get_input( channel ))
end



--- Input
input = {1,2}
for chan = 1, #input do
    input[chan] = Input.new( chan )
end

--- Output
output = {1,2,3,4}
for chan = 1, #output do
    output[chan] = Output.new( chan )
end

--- Asl
function toward_handler( id ) end -- do nothing if Asl not active
-- if defined, make sure active before setting up actions and banging
if Asl then
    toward_handler = function( id )
        output[id].asl:step()
    end
end
-- TODO should 'go_toward' be called 'slew'???
-- special wrapper should really be in the ASL lib itself?
function LL_toward( id, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    if type(s) == 'function' then s = s() end
    go_toward( id, d, t, s )
end

function LL_get_state( id )
    return get_state(id)
end


--- II default actions
--TODO int16 conversion should be rolled into i2c generation tool
II._c.input = function(chan)
    if chan == 1 or chan == 2 then
        return (1638.4 * input[chan]())
    else return 0 end
end

--TODO deprecate to the single `input` after format conversion added
II._c.inputF = function(chan)
    if chan == 1 or chan == 2 then return input[chan]()
    else return 0 end
end

--TODO int16 conversion should be rolled into i2c generation tool
II._c.output = function(chan,val)
    output[chan].level = val/1638.4
    --TODO step ASL
end

II._c.slew = function(chan,slew)
    output[chan].rate = slew/1000 -- ms
end

--- True Random Number Generator
-- redefine library function to use stm native rng
math.random = function(a,b)
    if a == nil then return random_get()
    elseif b == nil then return random_get() * a
    else return (b-a)*random_get() + a
    end
end

--- Flash program
function start_flash_chunk()
    -- should kill the currently running lua script
    -- turn off timers & callbacks? else?
    -- call to C to switch from REPL to writer
end


--- Syntax extensions
function closure_if_table( f )
    local _f = f
    return function( ... )
            if ... == nil then
                return _f()
            elseif type( ... ) == 'table' then
                local args = ...
                debug_usart('table')
                return function() return _f( table.unpack(args) ) end
            else return _f( ... ) end
        end
end
-- these functions are overloaded with the table->closure functionality
wrapped_fns = { 'math.random'
              , 'math.min'
              , 'math.max'
              }
-- this hack is required to change the identifier (eg math.random) itself(?)
for _,fn in ipairs( wrapped_fns ) do
    load( string.format('%s=closure_if_table(%s)',fn,fn))()
    -- below is original version that didn't work. nb: wrapped_fns was fns not strs
    -- fn = closure_if_table( fn ) -- this *doesn't* redirect the identifier
end

print'crowlib loaded'

-- cleanup all unused lua objects before releasing to the userscript
-- call twice to ensure all finalizers are caught
collectgarbage()
collectgarbage()

return _crow
