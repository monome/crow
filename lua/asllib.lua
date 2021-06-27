--- ASL library
-- a collection of ASL descriptors, converted from original asllib

Asllib = {}

-- note is deprecated for use in ASL
-- if you're sequencing notes in time, use Sequins & Clock
function note(mn, dur) return to(mn/12, dur, 'now') end

function lfo(time, level, shape)
    time = time or 1
    level = level or 5
    shape = shape or 'sine'

    return loop{ to( level, time/2, shape)
               , to(-level, time/2, shape)
               }
end

function oscillate(freq, level, shape)
    return lfo(1/freq, level, shape)
end

function pulse(time, level, polarity)
    time = time or 0.01
    level = level or 5
    polarity = polarity or 1

    return{ asl._if(polarity, { to(level, time, 'now')
                              , to(0, 0)
                              })
          , asl._if(1-polarity, { to(0, time, 'now')
                                , to(level, 0)
                                })
          }
end

function ramp(time, skew, level)
    time = time  or 1
    skew = skew or 0.25
    level = level or 5

    return loop{ to( level, time*(1.996*skew + 0.001))
               , to(-level, time*(1.998 - 1.996*skew))
               }
end

function ar(a, r, level, shape)
    a = a or 0.05
    r = r or 0.5
    level = level or 7
    shape = shape or 'log'

    return{ to(level, a, shape)
          , to(0, r, shape)
          }
end

function adsr(a, d, s, r, shape)
    a = a or 0.05
    d = d or 0.3
    s = s or 2
    r = r or 2

    return{ held{ to(7.0, a, shape)
                , to(s, d, shape)
                }
          , to(0, r, shape)
          }
end

return Asllib
