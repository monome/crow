function LL_toward( d, t, s )
    print("toward",d,"in time:",t,"with shape:",s)
end



-- A Slope Language
local asl = { program = {}     -- store the ASL program
            , retStk  = {}     -- return stack for nested constructs
            , pc      = 0      -- program counter within stack frame
            , hold    = false  -- is the slope trigger currently held high
            , in_hold = false  -- is the ASL program currently in a held construct
            , lock    = false  -- program sets to lockout bangs during lock{}
            }

local table = require("table")
local print = print

-- COMPILER
-- create a table with a sequence of functions

local function exit()
    asl.pc = asl.retStk[#asl.retStk]
    table.remove( asl.retStk )
end

-- deferred versions of intrinsics
local function q_exit() return exit end

local function compileT( aslT )
    local x = {}
    for k, fn in ipairs(aslT) do
        table.insert( x, fn )
    end
    table.insert( x, q_exit() )
    return x
end

function asl:set( aslT )
    asl.program  = compileT( aslT )
    asl.retStk   = {}
    asl.hold     = false
    asl.locked   = false
end

-- RUNTIME behaviour
-- relates bang() to internal AST representation

-- step through the program recursively!
-- this is stupid. we're searching the tree every step!
-- think about how to do this in a direct way

-- returns the current table context of the program counter
--> helper doASL()
local function get_frame()
    local frame = asl.program -- the table representing the program
    for i=1, #asl.retStk, 1 do
        frame = frame[asl.retStk[i]]
    end
    return frame
end

local function doASL()
    local p = get_frame()                     -- find table level
    if p[asl.pc] == nil then print"asl.pc == nil" return
    elseif type(p[asl.pc]) == "function" then
        wait = p[asl.pc]()                  -- execute the step
        if asl.pc ~= nil then
            asl.pc = asl.pc + 1
            if wait ~= "wait" then doASL() end  -- recurse for next step
        end -- else we did last exit()
    else
        table.insert( asl.retStk, asl.pc )
        asl.pc = 1
        doASL()                               -- unpack table & recurse
    end
end

function callback()
    doASL()
end

function set_hold_state(state)
    asl.hold = state
end

local function start()
    if asl.program ~= nil then
        asl.pc      = 1
        asl.retStk  = {}
    end
    doASL()
end

function release()
    asl.hold = false
    if asl.in_hold then
        asl.in_hold = false
        exit()
    end
end

-- replace pc = 0 with tag(tagname) goto(tagname) pairs
function recur() asl.pc = 0 end

-- public accessors
function asl:bang( dir )
    asl.hold = dir
    if asl.program ~= nil then
        if asl.locked ~= true then
            if dir == true then
                start( asl )
            else
                release( asl )
            end
        end
    end
end





-- hw access fn
function toward( dest, time, shape )
    local d,t,s = dest or 0, time or 1, shape or "linear"
    return function()
        LL_toward(d,t,s)
        if t ~= 0 then
            return "wait"
        end
    end
end

-- sugared toward(). could be variable arity instead?
function now( dest )
    return toward(d,0,LINEAR)
end

function delay( time )
    return toward( asl.here, t, LINEAR )
end





-- WRAPPING functions
function asl_if( fn_to_bool, aslT )
    local ftb = fn_to_bool
    t = compileT( aslT )
    table.insert( t, 1, function() if ftb() == false then exit() end end )
    return t
end

function q_recur() return recur end

function asl_wrap( enter_fn, aslProgram, exit_fn )
    p = aslProgram
    table.insert( p,  1, enter_fn )
    table.insert( p, #p, exit_fn )
    return p
end





-- first layer of abstraction over VM
function loop( aslT )
    t = compileT( aslT )
    t[#t] = q_recur() -- overwrite exit() with recur()
    return t
end

-- could abstract this form into asl_wrap()
function lock( aslT )
    return asl_wrap( function() asl.lock = true  end
                   , compileT( aslT )
                   , function() asl.lock = false end
                   )
end

function held( aslT )
--    return asl_if( function() return asl.hold end
--                 , aslT
--                 )

    return asl_wrap( function() asl.in_hold = true end
                   , asl_if( function() return asl.hold end
                           , aslT
                           )
                   , function() asl.in_hold = false end
                   )
end

function times( count, aslT )
    local n = count -- does this become an issue in the class? asl.n?
    t = asl_if( function() local result = n > 0
                           n = n - 1
                           return result end
              , aslT
              )
    t[#t] = q_recur()
    return t
end

return asl
