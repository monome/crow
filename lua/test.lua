-- must be provided for asl.lua
function LL_toward( id, d, t, s )
    print("id: "..id,"\ttoward "..d,"\tin time: "..t,"\twith shape: "..s)
end


local Asl = dofile("lua/asl.lua")

local function tester(sl)
    local shape = "sine"
    sl:action
        { now(0)                            -- reset LFO to 0
        , loop{ toward(  5, 2, shape ) -- LFO rises
              , toward( -5, 2, shape ) -- LFO falls
              }                             -- repeats rise&fall
        }
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

local function tester2(sl)
    sl:action( lfo() )
end

local function tester3()
    sl:action
        { now(3)
        , asl_if( function() return true end
                , { toward( -3 )
                  , toward( 4.5 )
                  }
                )
        , toward(4)
        }
end

local function tester4(sl)
    sl:action
        { toward(7)
        , times( 2, { toward(3)
                    , toward(-4)
                    }
               )
        , toward(-5)
        }
end

local function tester5(sl)
    sl:action
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

local sl = {}
sl[1] = Asl.new(1)
sl[2] = Asl.new(2)
tester5(sl[1])
sl[2]:action(lfo())
sl[1]:bang(true)
--set_hold_state(false)
sl[1]:callback()
sl[2]:bang(true)
sl[1]:callback()
sl[1]:bang(false)
sl[2]:callback()
sl[2]:callback()
sl[1]:callback()
sl[1]:callback()
sl[2]:callback()
sl[1]:callback()
sl[2]:callback()
sl[1]:callback()
--asl:bang(false)

-- desired representation of the above asl:action
-- { now(0)
-- , { toward( 5, 2, "sine")
--   , toward( -5, 2, "sine")
--   , restart_frame()
--   }
-- , exit(0)
-- }
--
--
--caller of exit

