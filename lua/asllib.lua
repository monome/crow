--- ASL library
--
-- a collection of basic asl scripts

Asllib = {}

function n2v( n ) return asl.runtime( function(a) return a/12 end, n ) end
function negate( n ) return asl.runtime( function(a) return -a end, n ) end
function clamp( input, min, max )
    min, max = min or 0.005, max or 1e10
    return asl.runtime( function( a, b, c )
                 return math.min( math.max( b, a ), c )
               end
             , input, min, max )
end

function note( noteNum, duration )
    return to( n2v(noteNum), duration, 'now' )
end

function lfo( time, level, shape )
    time, level = time or 1, level or 5

    local function half(t) return asl.runtime( function(a) return clamp(a/2) end, t ) end

    return loop{ to(        level , half(time), shape )
               , to( negate(level), half(time), shape )
               }

end

function pulse( time, level, polarity )
    time, level, polarity = time or 0.01, level or 5, polarity or 1

    local function active(mag,pol)
        return asl.runtime( function(a,b) return (b == 0) and 0 or a end
                 , mag, pol)
    end
    local function resting(mag,pol)
        return asl.runtime( function(a,b) return (b == 0) and a or 0 end
                 , mag, pol)
    end

    return{ to( active(level,polarity) , clamp(time), 'now' )
          , to( resting(level,polarity), 0 )
          }
end

function ramp( time, skew, level )
    time,skew,level = time  or 1
                    , skew  or 0.25
                    , level or 5

    time2 = clamp(time)
    skew2 = clamp(skew,0,1)

    local function riser(t,sk)
        return asl.runtime( function(a,b) return a*(1.996*b + 0.001) end
                 , t, sk)
    end

    local function faller(t,sk)
        return asl.runtime( function(a,b) return a*(1.998 - 1.996*b) end
                 , t, sk)
    end

    return{ loop{ to(        level , riser(time2,skew) )
                , to( negate(level), faller(time2,skew) )
                }
          }
end

function ar( attack, release, level, shape )
    attack,release,level = attack  or 0.05
                         , release or 0.5
                         , level   or 7

    return{ to( level, clamp(attack) , shape )
          , to( 0    , clamp(release), shape )
          }
end

function adsr( attack, decay, sustain, release, shape )
    attack,decay,sustain,release = attack  or 0.05
                                 , decay   or 0.3
                                 , sustain or 2
                                 , release or 2

    return{ held{ to( 5.0    , clamp(attack), shape )
                , to( sustain, clamp(decay) , shape  )
                }
          , to( 0, clamp(release), shape )
          }
end

return Asllib
