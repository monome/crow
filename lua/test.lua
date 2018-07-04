local asl = dofile("lua/asl.lua")

local function tester()
    local shape = "sine"
    asl:set
        { now(0)                            -- reset LFO to 0
        , loop{ toward(  5, 2, shape ) -- LFO rises
              , toward( -5, 2, shape ) -- LFO falls
              }                             -- repeats rise&fall
        }
    asl:bang(true) -- start the slope!
end

tester()
--asl:bang(false)

-- desired representation of the above asl:set
-- { now(0)
-- , { toward( 5, 2, "sine")
--   , toward( -5, 2, "sine")
--   , restart_frame()
--   }
-- , exit(0)
-- }
