-- dummy functions
function go_toward( id,d,t,s ) end



-- must be provided for asl.lua
function LL_toward( self, d, t, s )
    -- that would allow one to access the state of the output in a closure
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    print("id: "..self.id,"\ttoward "..d,"\tin time: "..t,"\twith shape: "..s)
    go_toward( self.id, d, t, s )
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

-- closured fns to be invoked fresh each time
function q_random( lower, upper )
    return function() return math.random(lower,upper) end
end

-- example of a 'library' shape
local function lfo( speed, curve, level )
    -- default values for variadic fn use
    speed = speed or 1
    curve = curve or "linear"
    level = level or 5

    return loop{ toward( q_random(-5,5), speed, curve )
               , toward( -level, speed, curve )
               }

end

local sl = {}
sl[1] = Asl.new(1)
sl[1]:action(lfo())
sl[1]:bang(true)
sl[1]:step()
sl[1]:step()
sl[1]:step()
sl[1]:step()
