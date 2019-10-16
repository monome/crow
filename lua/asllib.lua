--- ASL library
--
-- a collection of basic asl scripts
--
Asllib = {} -- how should we refer to the below?

-- clamp to bounds, must be greater than zero
function boundgz( n )
    if type(n) == 'function' then
        return function()
            local nn = n()
            return (nn <= 0.01) and 0.01 or nn
        end
    else return (n <= 0.01) and 0.01 or n end
end

function clamp(input, min, max)
	min = min or 0.001
	max = max or 1e10 -- picked this as an arbitrarily large number...
	if type(input) == 'function' then
		return function() return clamp(input(),min,max) end
    else 
		return math.min(math.max(min,input),max)
	end
end

function div(n,d)
    if type(n) == 'function' then
		if type(d) == 'function' then
			return function() return n()/d() end
		else
			return function() return n()/d end
		end
	elseif type(d) == 'function' then
		return function() return n/d() end
    else 
		return n/d
	end
end


function sub(a,b)
    if type(a) == 'function' then
		if type(b) == 'function' then
			return function() return a()-b() end
		else
			return function() return a()-b end
		end
	elseif type(b) == 'function' then
		return function() return a-b() end
    else 
		return a-b
	end
end

function mult(a,b)
    if type(a) == 'function' then
		if type(b) == 'function' then
			return function() return a()*b() end
		else
			return function() return a()*b end
		end
	elseif type(b) == 'function' then
		return function() return a*b() end
    else 
		return a*b
	end
end

function plus(a,b)
    if type(a) == 'function' then
		if type(b) == 'function' then
			return function() return a()+b() end
		else
			return function() return a()+b end
		end
	elseif type(b) == 'function' then
		return function() return a+b() end
    else 
		return a+b
	end
end

function negate( v )
    if type(v) == 'function' then
        return function() return -v() end
    else return -v end
end

function n2v( n )
    if type(n) == 'function' then
        return function () return n()/12 end
    else return n/12 end
end

function note( noteNum, duration )
    return{ to( n2v(noteNum), 0 )
          , to( 'here', duration  )
          }
end

function lfo( time, level )
    time, level = time or 1, level or 5
	time = clamp(time,0.006,1e10)
	halfTime = div(time,2)
    return loop{ to(        level , halfTime )
               , to( negate(level), halfTime )
               }

end

function pulse( time, level, polarity )
    time, level, polarity = time or 0.01, level or 5, polarity or 1
	time = clamp(time,0.006,1e10)
    local rest = 0
    if polarity == 0 then
        rest  = level
        level = 0
    end

    return{ to( level, 0 )
          , to( level, time )
          , to( rest , 0 )
          }
end


function ramp( time, skew, level ) 
    time,skew,level = time  or 1 
                    , skew  or 0.25 
                    , level or 5 
	
	time = clamp(time,0.006,1e10)
	skew = clamp(skew,0,1)
	
    local rise = div(0.5,plus(mult(skew,0.998),0.001))
    local fall = div(1.0,sub(2.0,div(1.0,rise)))
	local riseTime = div(time,rise) 
	local fallTime = div(time,fall)
	
    return{ loop{ to(  level, riseTime ) 
                , to( negate(level), fallTime ) 
                } 
          } 
end 

function ar( attack, release, level )
    attack,release,level = attack  or 0.05
                         , release or 0.5
                         , level   or 7

    attack = clamp(attack,0.002,1e10)
	release = clamp(release,0.002,1e10)
	return{ to( level, attack )
          , to( 0,     release )
          }
end

function adsr( attack, decay, sustain, release )
    attack,decay,sustain,release = attack  or 0.05
                                 , decay   or 0.3
                                 , sustain or 2
                                 , release or 2

    
	attack = clamp(attack,0.002,1e10)
	decay = clamp(decay,0.002,1e10)
	release = clamp(release,0.002,1e10)
	
	return{ held{ to( 5.0, attack )
                , to( sustain, decay )
                }
          , to( 0, release )
          }
end

print 'asllib loaded'

return Asllib
