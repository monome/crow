-- A Slope Language
--
-- A tiny DSL for describing envelopes and LFOs for musical applications
-- Focused on declarative syntax, and providing an abstraction layer
-- between hardware implementation of slewing & shaping.
--
-- Values can themselves be expressions (eg: time/2, rand() )
-- These expressions are evaluated lazily (ie at execution time)
-- Some method to force evaluation at compile time could be implemented
--
-- Meta-functions are implemented as functions that take ASL tables


-- EXAMPLE DESCRIPTIONS (in OUTPUT library):
--   function lfo( time, shape )
--       self.asl:set
--           { now(0)                            -- reset LFO to 0
--           , loop{ toward(  5, time/2, shape ) -- LFO rises
--                 , toward( -5, time/2, shape ) -- LFO falls
--                 }                             -- repeats rise&fall
--           }
--       self.asl:bang(1) -- start the slope!
--   end
--   
--   function trig( attack, decay, shape )
--       self.asl:set
--           { now(0)                           -- reset to 0
--           , lock{ toward( 5, attack, shape ) -- attack phase
--                 , toward( 0, decay,  shape ) -- decay phase (w sustain level)
--                 }                            -- must reach here before retrigger
--           }
--   end
--   
--   function adsr( a, d, s, r, shape)
--       self.asl:set
--           { held{ toward( 5, a, shape ) -- attack phase
--                 , toward( s, d, shape ) -- decay->sustain
--                 }                       -- hold at sustain
--           , toward( 0, r, shape )       -- release on !hold
--           }
--   end
--   
--   -- obviously it becomes very easy to expand on these simple forms
--   -- these could easily be wrapped to use time/ramp parameters instead
--   -- or they could be overloaded to add custom voltage heights
--   
--   -- more basically, the commands can be nested for more complex shapes
--   function custom()
--       self.asl:set
--           { now(0)                                   -- reset to 0
--           , held{ toward( 5, 1000, LOG ) }           -- slew to 5, wait til gate=0
--           , lock{ times{ 3, { toward( 1, 100, SINE ) -- slew to 1
--                             , delay( 50 )            -- wait 50ms
--                             , toward( 2, 100, SINE ) -- slew to 2
--                             }                        -- repeat last 3 lines
--                        }                             --    ^3 times
--                 , now(5)                             -- jump to 5
--                 }                                    -- triggers are ignored until now
--           , loop{ toward( 0, 1000, LINEAR )          -- slew to 0
--                 , delay(100)                         -- wait at 0
--                 , now(5)                             -- jump to 5
--                 }                                    -- repeat until new bang or clear
--           }
--   end
--   

-- LOW-LEVEL IMPLEMENTATION:

-- Importantly these are just syntactic wrappers over the control flow
-- The low-level implementation need only implement the toward fn

local inspect = require('inspect')

function LL_toward( d, t, s )
    -- from the current location
    -- goto value d
    -- in time t
    -- with shaping s
    print("toward",d,"in time:",t,"with shape:",s)
end

-- The following may be implemented directly
-- If not, the below weak functions will be used

--function now( d )
--    -- goto value d immediately
--    toward( d, 0, LINEAR )
--end

--function delay( t )
--    -- wait time t before executing the next command
--    -- negative t should wait forever
--    toward( self.here, t, LINEAR )
--end

-- Additionally, the LL driver should provide a callback on completion of toward()

callback = function()
    -- process the next stage of the program
    --asl:doASL()
end

-- Init can check if 'callback' is set to nil to know whether an ASL
-- callback is provided
-- If no callback, ASL can create it's own timer & assume actions are
-- completed on time

-- OUTER CLASS MODIFICATIONS:
-- The output class shall provide the following metadata
--    out = { here = 0      -- current value of the output (this could be a fnptr!)
--          , asl  = {}     -- table to store an active ASL. nil'd if none
--          }
--    
--    function clear()
--        self.asl = {}
--        asl.now( self.here ) -- freeze ongoing slope at it's current location
--        -- caller of this method can customize action with their own now()
--        -- or toward() call
--    end
--    
--    


-- begin ASL module!
local asl = {}

local table = require("table")
local print = print

-- Internal data structure
asl = { program = {}     -- store the ASL program
      , retStk  = {}     -- return stack for nested constructs
      , pc      = 0      -- program counter within stack frame
      , hold    = false  -- is the slope trigger currently held high
      , lock    = false  -- program sets to lockout bangs during lock{}
      }

local function exit()
    return function()
        table.remove( asl.retStk )
        asl.pc = asl.retStk[#asl.retStk]
    end
end
-- COMPILER
-- convert the syntactic sugar of ASL into tokenized form so it can be traversed
-- basically how forth turns IF ELSE THEN into BRANCH QBRANCH
-- or an infinite loop becomes BEGIN AGAIN

-- create a table with a sequence of functions
local function compileT( aslT )
    local x = {}
    for k, fn in ipairs(aslT) do
        table.insert( x, fn )
    end
    table.insert( x, exit() )
    return x
end

function asl:set( aslT )
    asl.program = compileT( aslT )
    asl.retStk  = {}
    asl.hold    = false
    asl.locked  = false
    --print(inspect(asl.program))
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
    -- TODO: bring get_frame() into here!
    local p = get_frame()                     -- find table level
    if p[asl.pc] == nil then return           -- we're done
    elseif type(p[asl.pc]) == "function" then
        wait = p[asl.pc]()                -- execute the step
        asl.pc = asl.pc + 1
        if wait ~= "wait" then doASL() end  -- recurse for next step
    else
        table.insert( asl.retStk, asl.pc )
        asl.pc = 1
        doASL()                               -- unpack table & recurse
    end
end

function callback()
    -- process the next stage of the program
    doASL()
end

local function start()
    if asl.program ~= nil then
        asl.pc      = 1
        asl.retStk  = {}
    end
    doASL()
end

local function drop()
    print "bdrop"
    -- check if we're stuck in a held{} construct
    -- if not: progress straight to the next section
end

function asl:bang( dir )
    asl.hold = dir
    if asl.program ~= nil then
        if asl.locked ~= true then
            if dir == true then
                start( asl )
            else
                drop( asl )
            end
        end
    end
end


-- The primary purpose of this syntax is to abstract any handling of the callback
-- directly. Rather, the user can use the following constructs to define sequences
-- of slopes, and their interaction with output/hardware state.
--
-- These constructs are implemented as functions that take ASL tables, thus are nestable


-- TODO: GLOBAL
function toward( dest, time, shape )
    -- from the current location
    -- goto value d
    -- in time t
    -- with shaping s

    -- create a closure containing the low-level call along with it's params
    local d,t,s = dest, time, shape
    return function()
        LL_toward(d,t,s)
        if t ~= 0 then
            return "wait"
        end
    end
end

-- goto value d immediately
-- TODO:GLOBAL
function now( dest )
    local d=dest
    return toward(d,0,LINEAR)
end

-- wait time t before executing the next command
-- TODO: GLOBAL
function delay( time )
    local t=time
    return toward( asl.here, t, LINEAR )
end


-- WRAPPING functions
-- these don't need to be public right?
-- the ASL table that gets sent in is really just a string?
-- TODO: GLOBAL
function loop( aslT )
    -- executes the aslT forever
        -- TODO: restart_frame() should be defined in here
    t = compileT( aslT )
    -- now overwrite normal exit() by resetting the program-counter
    t[#t] = function() asl.pc = 0 end
    return t
end

local function lockSet(state)
    local s = state
    return function() asl.lock = s end
end
-- TODO: GLOBAL
function lock( aslT )
    -- performs the aslT
    -- all bang/level commands are ignored until aslt complete
        -- TODO: can define lockSet as a local function in here
    t = compileT( aslT )
    table.insert( t, 1, function() lockSet(true) end ) -- first
    table.insert( t, #t, function() lockSet(false) end ) -- before exit()
    return t
end

-- TODO: pull side held()
local function end_hold() end -- just a placeholder to search for
local function q_skip_hold() end -- conditional search for the placeholder

-- TODO: GLOBAL
function held( aslT )
    -- performs the aslT
    -- if the 'held' state variable is unset, the next command will
    -- be executed immediately
    -- 'held' represents a high 'gate' signal in synthesizer terms
    t = compileT( aslT )
    table.insert( t, 1, q_skip_hold() )
    table.insert( t, #t, delay(-1) )
    table.insert( t, #t, end_hold() ) -- incase control doesn't reach
    return t
end

-- iterate n times
function q_countdown( count ) -- helper to decrement counter
    local countdown = count
    return function()
        if countdown > 0 then
            countdown = countdown - 1
            return
        else
            -- skip .pc to exit() at the current level
            return
        end
    end
end
-- TODO: GLOBAL
function times( count_aslT )
    t = compileT( aslT )
    table.insert( t, 1, q_countdown( count_aslT[1]) )
    return t
end

return asl
