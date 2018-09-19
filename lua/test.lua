-- must be provided for asl.lua
function LL_toward( d, t, s )
    print("toward",d,"in time:",t,"with shape:",s)
end


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

-- example of a 'library' shape
local function lfo( speed, curve, level )
    -- default values for variadic fn use
    speed = speed or 1
    curve = curve or "linear"
    level = level or 5

    return { loop{ toward(  level, speed, curve )
                 , toward( -level, speed, curve )
                 }
           }
end

local function tester2()
    asl:set( lfo() )
end

local function tester3()
    asl:set
        { now(3)
        , asl_if( function() return true end
                , { toward( -3 )
                  , toward( 4.5 )
                  }
                )
        , toward(4)
        }
end

local function tester4()
    asl:set
        { toward(7)
        , times( 2, { toward(3)
                    , toward(-4)
                    }
               )
        , toward(-5)
        }
end

local function tester5()
    asl:set
        { toward(5)
        , held{ toward(4)
              , toward(3)
              , toward(2)
              , toward(1)
              , toward(0)
              }
        , toward(-1)
        , toward(-2)
        , toward(-3)
        , toward(-4)
        }
end

tester5()
asl:bang(true)
--set_hold_state(false)
callback()
asl:bang(true)
callback()
callback()
asl:bang(false)
callback()
callback()
callback()
callback()
callback()
callback()
callback()
callback()
callback()
callback()

--asl:bang(false)

-- desired representation of the above asl:set
-- { now(0)
-- , { toward( 5, 2, "sine")
--   , toward( -5, 2, "sine")
--   , restart_frame()
--   }
-- , exit(0)
-- }
