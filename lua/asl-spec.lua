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

--local inspect = require('inspect')

function LL_toward( d, t, s )
    -- from the current location
    -- goto value d
    -- in time t
    -- with shaping s
    --
    -- add polymorphic handling of parameters held in closure
    -- this allows deferred or pre-compiled values
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



