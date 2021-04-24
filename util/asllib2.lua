--- asl2 library
-- this will become asllib, but we run it here to work on it without recompiling

function lfo( time, level, shape )
    time  = time  or 1
    level = level or 5
    shape = shape or 'sine'

    return loop{ to(  level, time/2, shape )
               , to( -level, time/2, shape )
               }
end
-- output[1]( lfo() )
-- output[1]( lfo(0.005, 3) )
-- output[1]( lfo(0.005, 3, 'expo') )


-- TODO
-- handle iterables in C:describe
output[1].action = asl._while(iterable(4), {to(4, 0.01), to(-4, 0.1)})
-- this compiles ok, but triggers a hardfault on execution
-- output[1]( asl._while(iterable(4), {to(4, 0.01), to(-4, 0.1)}))
-- output[1]( times(20, {to(4, 0.01), to(-4, 0.1)}))


-- TODO
-- enable math ops for dynamics (should just be overloading Dynmt with Itermt?)


-- TODO
-- slopes run at half speed for some reason?


