--- A Slope Language
--
-- A tiny DSL for describing envelopes and LFOs for musical applications
-- Focused on declarative syntax, and providing an abstraction layer
-- between hardware implementation of slewing & shaping.
--
-- Values can themselves be expressions (eg: time/2, rand() )
-- Expressions are evaluated at runtime, so closures allow actions to respond to
-- live changes of system state.


--- Basic usage
--
-- A number of small


--- Standard library
--
--


--- Implementing ASL on alternate platforms
--
-- On crow, the user shouldn't need to think about these details, but if porting
-- ASL to run on another platform, the developer must implement the 'toward' fn
-- and a callback on reaching the breakpoint.
--
-- LL_toward is the atomic unit of ASL, representing a transition from the current
-- state to the 'destination' value, over a set 'time' period, while being shaped
-- by some 'shape'. On crow 'shape' has a predetermined set of possibilities, though
-- this is liable to be extended on crow, or in other manifestations.

@id      integer    index of the Asl. typically linked to a physical output
@dest    float      destination (typically volts) of the slope
@time    float      time (typically in seconds) the slope will take to complete
@shape   string     manipulation of the slope path (implementation specific)

LL_toward( id, destination, time, shape )

-- The requisite breakpoint callback must call `my_asl:step()` where my_asl is the
-- Asl object created by the Asl.new constructor. It shall be called whenever the
-- slope has completed in time. This action will trigger the next stage of the Asl,
-- or end the slope if no further stages exist. The callback shall return the id
-- matching that sent to LL_toward so the step() method can be called on the
-- appropriate ASL object.
--
-- nb: calls where time==0 should not raise a callback to ASL, as the
-- 'go right now' behaviour is handled internally by ASL.
--
-- TODO: add details about getter for 'held'


--- INTERNALS
--
-- A short walkthrough of the implementation in Lua. Studying this library will
-- give a good real-world introduction to using coroutines to solve asynchronous
-- problems. Such an approach is particularly useful for implementing musical
-- structures as we often are dealing with multiple levels of abstraction
-- operating on different (and varying) time scales.
--
-- Many thanks to good friend Veit Heller (github.com/hellerve) who guided the
-- transformation to the coroutine based interpreter.


--- toward
--
-- The user function toward() is the user-facing building block of an ASL script.
-- The whole purpose of ASL is to allow automatic calling of a sequence of such
-- toward() at precisely the right moment. A simplified form is:

function toward( ... )
    return coroutine.create( function( self )
        while true do
            LL_toward( self.id, ... )
            coroutine.yield( (t ~= 0) and 'wait' or nil )
        end
    end)
end

-- At its core, this function takes a description of a slope (...) and returns a
-- coroutine (coroutine.create). This coroutine contains a function (actually a
-- closure), whose main purpose is to call the LL_toward() function with the
-- parameters received.
--
-- In order to call LL_toward() correctly, we need to know which Asl is calling
-- so we add self.id as the first argument. In order to gain access to self, we
-- send it *to the coroutine* when we call coroutine.resume. This is important
-- as if toward() itself required self, the user writing an ASL descriptor would
-- have to prefix every call with 'self'. We'll handle this self passing below.
--
-- After calling LL_toward, we call:

coroutine.yield( (t ~= 0) and true or false ) -- 't' is time in '...'

-- coroutine.yield() puts the coroutine to sleep, and allows us to a provide a
-- return-style value to the coroutine caller. This value is either true or false
-- depending on whether this stage should wait for a callback. In essence
-- this is just an optimization, meaning the low-level implementation need not
-- immediately callback, as it reaches the destination instantly.
--
-- The final element here is the wrapping of the whole function in a `while true`
-- construct. When we reach coroutine.yield, the coroutine is put to sleep, but
-- not killed. If the user then calls coroutine.resume the coroutine continues from
-- exactly where it yielded, thus without the loop, the second time the coroutine
-- was resumed, it would immediately return from the function, and the coroutine
-- would die.
--
-- ASL descriptors are designed to be called multiple times, so the `while true`
-- construct allows us to say 'every time we resume this coroutine, run the contents
-- of the while-construct once'. Thus a single coroutine can be called repeatedly
-- rather than needing to create a new coroutine to be executed each time.


--- set_action
--
-- Before delving into the ASL constructs, it's informative to understand how to
-- execute a plain sequence of toward structures. Take the following sequence which
-- ramps up to 1, then back to 0:

my_asl.set_action{ toward(1,1,'linear')
                 , toward(0,1,'linear')
                 }

-- NB: we're using a table-call with the curly braces, meaning we're sending a
-- literal table as our function argument. From above we know that our table
-- consists of two coroutines as created by the two calls to toward.
--
-- Internally the ASL is stored in my_asl.co, which naturally is a coroutine, so
-- for anything more than a single toward call, we will have to wrap our table of
-- coroutines into an outer coroutine that can step through our sequence. This
-- is certainly overkill for a simple case like the above, but it is the nesting
-- of coroutines that enables the ASL constructs below. We do it like so:

self.co = coroutine.create(function( self )
    while true do
        seq_coroutines( self, coroutine ) -- coroutine is a *table* of coroutines
    end
end)

-- Again we use the wrapping `while true` to ensure the ASL can be run multiple times.
-- The `fns` argument to seq_coroutines is a little misleading - it's actually a table
-- of coroutines, as is clear from the above call.

function seq_coroutines( self, fns )
    for i=1, #fns, 1 do
        local _,wait = coroutine.resume( fns[i], self )
        coroutine.yield( wait )
    end
end

-- In essence this function will call the first coroutine in the table, which will
-- yield a result into `wait`. We then take that yielded value, *and yield again*
-- to the outer context. This wrapped resume/yield pair is called until all elements
-- of the sequence have completed.
--
-- NB: The set_action function will only do this coroutine wrapping if it receives a
-- table, which is actually more of an edge case. Instead the ASL constructs will
-- return coroutines, which are assigned to self.co directly.


--- step
--
-- Now that we have a model of the underlying structure, we need a mechanism to
-- step through our coroutines. That function is called *step*, though it is
-- never called directly by the user. Instead the user will call `my_asl:do_action()`
-- to begin the ASL, and the low-level implementation will subsequently call
-- *step* through it's callback. This call happens when the current slope completes
-- and we want to progress to the next stage of the ASL.

function Asl:step()
    _,wait = coroutine.resume( self.co, self )
    if wait ~= true then self:step() end
end

-- This function is delightfully simple, it simply resumes the coroutine in self.co
-- yielding our 'wait' value. For optimization's sake, we immediately recur if
-- the coroutine yielded a 'false' value, meaning it's action was instanteous.


--- constructs: loop
--
-- loop{} is the simplest of the user-facing ASL constructs. It allows the user
-- to describe a structure that will repeat indefinitely. Expanding on the above
-- example

my_asl.set_action{ toward(1,1,'linear')
                 , toward(0,1,'linear')
                 }

--- constructs: asl_wrap
--
--
--- lock{}

--- constructs: asl_if
--
--
--- held{}
--- times{times, ...}


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
