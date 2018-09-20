crow = dofile('lua/crowlib.lua')
Asl = dofile('lua/asl.lua')

--TODO this should be hidden from the user altogether (inside 'out' table)
local slope = {}

-- TODO where should this go?
function LL_toward( self, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    --print('id '..self.id,'\ttoward '..d,'\tin time '..t,'\twith shape '..s)
    go_toward( self.id, d, t, s )
end

function toward_handler( id )
    slope[id]:step()
end


-- TODO where should these go?
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

    --local slope = {}
    slope[1] = Asl.new(1)
    slope[1]:action(lfo())
    slope[1]:bang(true)

    print(crow.squared(5))
end
