-- expects 'asl:lfo()'
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

-- call in ur program to use
mySlope:set(lfo(1, "sine", 3))



