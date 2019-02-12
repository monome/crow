--- Crow standard library

local _crow = {}
local _c = _crow -- alias

--- System functions

_crow.version = '0.0.0'

function whatversion()
    return _crow.version
end

function printversion()
    print( 'crow ' .. whatversion() )
end


--- Library loader
--
local function closelibs()
    -- set whole list of libs to nil to close them
    -- TODO does this free the RAM used by 'dofile'?
    input  = nil
    output = nil
    asl    = nil
    asllib = nil
    metro  = nil
    ii     = nil
end

function _crow.libs( lib )
    if lib == nil then
        -- load all
        input  = dofile('lua/input.lua')
        output = dofile('lua/output.lua')
        asl    = dofile('lua/asl.lua')
        asllib = dofile('lua/asllib.lua')
        metro  = dofile('lua/metro.lua')
        ii     = dofile('lua/ii.lua')
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

function _crow.make_cmd( event_name, ... )
    local out_string = string.format('^^%s(',event_name)
    local args = {...}
    local arg_len = #args
    if arg_len > 0 then
        for i=1,arg_len-1 do
            out_string = out_string .. args[i] .. ',' -- can't use .format bc ?type
        end
        out_string = out_string .. args[arg_len]
    end
    return out_string .. ')'
end

function _crow.tell( event_name, ... )
    print(_crow.make_cmd( event_name, ... ))
end

function get_out( channel ) _crow.tell( 'out_cv', channel, get_state(channel)) end
--function get_cv( channel )  _crow.tell( 'ret_cv', channel, io_get_input(channel)) end
function get_cv( channel )
    _c.tell('ret_cv',channel,io_get_input( channel ))
end



--- Input
input = {1,2}
for chan = 1, #input do
    input[chan] = input.new( chan )
end

--- Output
out = {1,2,3,4}
for chan = 1, #out do
    out[chan] = output.new( chan )
end

--- Asl
function toward_handler( id ) end -- do nothing if Asl not active
-- if defined, make sure active before setting up actions and banging
if asl then
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
    go_toward( id, d, t, s )
end

function LL_get_state( id )
    return get_state(id)
end



adc_remote = function(chan)
    get_cv(chan)
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
            if type( ... ) == 'table' then
                local args = ...
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

return _crow
