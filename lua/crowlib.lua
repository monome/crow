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
    asl    = nil
    asllib = nil
    metro  = nil
    ii     = nil
    cal    = nil
    midi   = nil
end

function _crow.libs( lib )
    if lib == nil then
        -- load all
        Input  = dofile('lua/input.lua')
        Output = dofile('lua/output.lua')
        asl    = dofile('lua/asl.lua')
        asllib = dofile('lua/asllib.lua')
        metro  = dofile('lua/metro.lua')
        clock  = dofile('lua/clock.lua')
        ii     = dofile('lua/ii.lua')
        cal    = dofile('lua/calibrate.lua')
        --midi   = dofile('lua/midi.lua')
    elseif type(lib) == 'table' then
        -- load the list
    else
        if lib == 'close' then closelibs() end
        -- assume string & load single library
    end
end

-- open all libs by default
_crow.libs()

function _crow.reset()
    for n=1,2 do
        input[n].mode = 'none'
        input[n]:reset_events()
    end
    for n=1,4 do
        output[n].slew = 0
        output[n].volts = 0
        output[n].scale('none')
    end
    ii.reset_events(ii.self)
    ii_follow_reset() -- resets forwarding to output libs
    metro.free_all()
    clock.cleanup()
end

--- Communication functions
-- these will be called from norns (or the REPL)
-- they return values wrapped in strings that can be used in Lua directly
-- via dostring

--TODO tell should be in c-fns table, not _crow table?
function _crow.tell( event_name, ... )
    tell( event_name, ... )
end

function get_out( channel )
    _c.tell( 'output', channel, get_state( channel ))
end
function get_cv( channel )
    _c.tell( 'stream', channel, io_get_input( channel ))
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


--- asl
function toward_handler( id ) end -- do nothing if asl not active
-- if defined, make sure active before setting up actions and banging
if asl then
    toward_handler = function( id )
        output[id].asl:step()
    end
end
-- special wrapper should really be in the ASL lib itself?
function LL_toward( id, d, t, s )
    while type(d) == 'function' do d = d() end
    while type(t) == 'function' do t = t() end
    while type(s) == 'function' do s = s() end
    go_toward( id, d, t, s )
end

function LL_get_state( id )
    return get_state(id)
end


--- ii
-- pullups on by default
ii.pullup(true)


--- follower default actions
function ii_follow_reset()
    ii.self.volts = function(chan,val) output[chan].volts = val end
    ii.self.slew = function(chan,slew) output[chan].slew = slew end
    ii.self.reset = function() _crow.reset() end
    ii.self.pulse = function(chan,ms,volts,pol) output[chan](pulse(ms,volts,pol)) end
    ii.self.ar = function(chan,atk,rel,volts) output[chan](ar(atk,rel,volts)) end
    -- convert freq to seconds where freq==0 is 1Hz
    ii.self.lfo = function(chan,freq,level,skew) output[chan](ramp(math.pow(2,-freq),skew,level)) end
end
ii_follow_reset()


--- True Random Number Generator
-- redefine library function to use stm native rng
math.random = function(a,b)
    if a == nil then return random_float()
    elseif b == nil then return random_int(1,a)
    else return random_int(a,b)
    end
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

-- allows 'calling' a fn arg, only if the arg is non-nil
-- just useful to avoid a nil check before calling an optional event handler
function safe_call(fn,...) if fn then return fn(...) end end


--- Delay execution of a function
-- dynamically assigns metros (clashes with indexed metro syntax)
function delay(action, time, repeats)
    local r = repeats or 0
    local d = {}
    function devent(c)
        action(c) -- make the action aware of current iteration
        if c > r then
            metro.free(d.id)
            d = nil
        end
    end
    d = metro.init(devent, time)
    if d then d:start() end
    return d
end

--- Just Intonation helpers
-- convert a single fraction, or table of fractions to just intonation
-- optional 'offset' is itself a just ratio
-- justvolts converts to volts-per-octave
-- just12 converts to 12TET representation (for *.scale libs)
-- just12 will convert a fraction or table of fractions into 12tet 'semitones'
function _justint(fn, f, off)
    off = off and fn(off) or 0 -- optional offset is a just ratio
    if type(f) == 'table' then
        local t = {}
        for k,v in ipairs(f) do
            t[k] = fn(v) + off
        end
        return t
    else -- assume number
        return fn(f) + off
    end
end
JIVOLT = 1 / math.log(2)
JI12TET = 12 * JIVOLT
function _jiv(f) return math.log(f) * JIVOLT end
function _ji12(f) return math.log(f) * JI12TET end
-- public functions
function justvolts(f, off) return _justint(_jiv, f, off) end
function just12(f, off) return _justint(_ji12, f, off) end
function hztovolts(hz, ref)
    ref = ref or 261.63 -- optional. defaults to middle-C
    return justvolts(hz/ref)
end

-- empty init function in case userscript doesn't define it
function init() end

-- cleanup all unused lua objects before releasing to the userscript
-- call twice to ensure all finalizers are caught
collectgarbage()
collectgarbage()

return _crow
