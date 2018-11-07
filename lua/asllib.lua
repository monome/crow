--- ASL library
--
-- a collection of basic asl scripts
--
local function lfo( speed, curve, level )
    -- allow these defaults to be attributes of the out channel
    speed, curve, level = speed or 1, curve or 'linear', level or 5

    return{ loop{ toward(  level, speed, curve )
                , toward( -level, speed, curve )
                }
          }
end

local function ramp( time, skew, curve, level )
    time,skew,curve,level = time  or 1
                          , skew  or 0.5
                          , curve or 'linear'
                          , level or 5

    -- note skew expects 0-1 range
	local rise = 0.5/(0.998 * skew + 0.001)
	local fall = 1.0/(2.0 - 1.0/rise)

    return{ loop{ toward(  level, time*rise, curve )
                , toward( -level, time*fall, curve )
                }
          }
end

local function ar( attack, release, curve, level )
    attack,release,level = attack  or 1
                         , release or 1
                         , curve   or 'linear'
                         , level   or 5

    return{ toward( level, attack,  curve )
          , toward( 0,     release, curve )
          }
end

-- continue for the following shapes
-- ASR
-- ADSR
-- HADSR
-- DADSR
-- trapezoidal
-- whatever other classic shapes there are???
