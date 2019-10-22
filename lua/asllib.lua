--- ASL library
--
-- a collection of basic asl scripts

Asllib = {}

-- apply a function of n-args to either literal values or return a
-- closure to be computed when the value is called.
function m1( fn, a, ... )
    if type(a) == 'function' then
        local t = {...}
        return function() return fn( a(), table.unpack(t) ) end
    else
        return fn( a, ... )
    end
end

-- one-closurable-arg functions
function n2v( n ) return m1( function(a) return a/12 end, n ) end
function negate( n ) return m1( function(a) return -a end, n ) end
function clamp( input, min, max )
    min, max = min or 0.005, max or 1e10
    return m1( function( a, b, c )
                 return math.min( math.max( b, a ), c )
               end
             , input, min, max )
end

-- apply a 2-arg function to literal or function arguments
-- use this to allow execution of a function to happen at runtime
-- additonal args are passed through, but literal only
-- TODO build this recursively with m1 above for n-args
function m2( fn, a, b, ... )
    if type(a) == 'function' then
        local t = {...}
        if type(b) == 'function' then
            return function() return fn(a(),b(),table.unpack(t)) end
        else
            return function() return fn(a(),b,table.unpack(t)) end
        end
    elseif type(b) == 'function' then
        local t = {...}
        return function() return fn(a,b(),table.unpack(t)) end
    else
        return fn(a,b,...)
    end
end

function note( noteNum, duration )
    return{ to( n2v(noteNum), 0 )
          , to( 'here', duration  )
          }
end

function lfo( time, level )
    time, level = time or 1, level or 5
    local function half(t) return m1( function(a) return clamp(a/2) end, t ) end


    return loop{ to(        level , half(time) )
               , to( negate(level), half(time) )}

end

function pulse( time, level, polarity )
    time, level, polarity = time or 0.01, level or 5, polarity or 1

    local function active(mag,pol)
        return m2( function(a,b) return (b == 0) and 0 or a end
                 , mag, pol)
    end
    local function resting(mag,pol)
        return m2( function(a,b) return (b == 0) and a or 0 end
                 , mag, pol)
    end

    return{ to( active(level,polarity) , 0 )
          , to( 'here'                 , clamp(time) )
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
        return m2( function(a,b) return a*(1.996*b + 0.001) end
                 , t, sk)
    end

    local function faller(t,sk)
        return m2( function(a,b) return a*(1.998 - 1.996*b) end
                 , t, sk)
    end

    return{ loop{ to(        level , riser(time,skew) )
                , to( negate(level), faller(time,skew) )
                }
          }
end

function ar( attack, release, level )
    attack,release,level = attack  or 0.05
                         , release or 0.5
                         , level   or 7

    return{ to( level, clamp(attack)  )
          , to( 0,     clamp(release) )
          }
end

function adsr( attack, decay, sustain, release )
    attack,decay,sustain,release = attack  or 0.05
                                 , decay   or 0.3
                                 , sustain or 2
                                 , release or 2

    return{ held{ to( 5.0    , clamp(attack) )
                , to( sustain, clamp(decay)  )
                }
          , to( 0, clamp(release) )
          }
end

return Asllib
