--- ASL library
--
-- a collection of basic asl scripts
--
Asllib = {} -- how should we refer to the below?

-- is it possible to overload the `-` operator with a metatable?
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
		  , to( 'here', duration )
	   	  }
end

function lfo( speed, curve, level )
    -- allow these defaults to be attributes of the out channel
    speed, curve, level = speed or 1, curve or 'linear', level or 5

    return loop{ to(        level , speed, curve )
               , to( negate(level), speed, curve )
               }

end

function trig( polarity, time, level )
    polarity, time, level = polarity or 1, time or 0.1, level or 5

    local rest = 0
    if not polarity then
        rest  = level
        level = 0
    end

    return{ to{ ['now'   ] = level }
          , to{ ['delay' ] = time  }
          , to{ ['now'   ] = rest  }
          }
end


function ramp( time, skew, curve, level )
    time,skew,curve,level = time  or 1
                          , skew  or 0.5
                          , curve or 'linear'
                          , level or 5

    -- note skew expects 0-1 range
	local rise = 0.5/(0.998 * skew + 0.001)
	local fall = 1.0/(2.0 - 1.0/rise)

    return{ loop{ to(  level, time*rise, curve )
                , to( -level, time*fall, curve )
                }
          }
end

function ar( attack, release, curve, level )
    attack,release,level = attack  or 1
                         , release or 1
                         , curve   or 'linear'
                         , level   or 5

    return{ to( level, attack,  curve )
          , to( 0,     release, curve )
          }
end

function adsr( attack, decay, sustain, release )
    attack,decay,sustain,release = attack  or 0.1
                                 , decay   or 0.5
                                 , sustain or 2
                                 , release or 4

    return{ held{ to( 5.0, attack )
                , to( sustain, decay )
                }
          , to( 0, release )
          }
end

print 'asllib loaded'

return Asllib

-- continue for the following shapes
-- ASR
-- HADSR
-- DADSR
-- trapezoidal
-- whatever other classic shapes there are???
