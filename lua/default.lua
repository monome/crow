crab = dofile('lua/crowlib.lua')
Asl = dofile('lua/asl.lua')

function LL_toward( self, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    --print('id '..self.id,'\ttoward '..d,'\tin time '..t,'\twith shape '..s)
    go_toward( self.id, d, t, s )
end

local function lfo( speed, curve, level )
    speed = speed or 1
    curve = curve or 'linear'
    level = level or 5

    return { loop{ toward(  level, speed, curve )
                 , toward( -level, speed, curve )
                 }
           }
end

function init()
    print'init()'

    local slope = {}
    slope[1] = Asl.new(1)
    slope[1]:action(lfo())
    slope[1]:bang(true)
    slope[1]:callback()

    print(crab.squared(5))
end
