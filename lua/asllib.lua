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
          , to( 'here', duration  )
          }
end

function lfo( speed, level )
    -- allow these defaults to be attributes of the out channel
    speed, level = speed or 1, level or 5

    return loop{ to(        level , speed )
               , to( negate(level), speed )
               }

end

function pulse( time, level, polarity )
    time, level, polarity = time or 0.01, level or 5, polarity or 1

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

    -- note skew expects 0-1 range
    local rise = 0.5/(0.998 * skew + 0.001)
    local fall = 1.0/(2.0 - 1.0/rise)

    return{ loop{ to(  level, time/rise )
                , to( -level, time/fall )
                }
          }
end

function ar( attack, release, level )
    attack,release,level = attack  or 0.05
                         , release or 0.5
                         , level   or 7

    return{ to( level, attack )
          , to( 0,     release )
          }
end

function adsr( attack, decay, sustain, release )
    attack,decay,sustain,release = attack  or 0.05
                                 , decay   or 0.3
                                 , sustain or 2
                                 , release or 2

    return{ held{ to( 5.0, attack )
                , to( sustain, decay )
                }
          , to( 0, release )
          }
end

print 'asllib loaded'

return Asllib
