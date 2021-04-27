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
-- aa = {times(2, {to(3, 0.1), to(-4, 0.1)}), to(0,0)}
-- aa = {times(4, {to(dyn{lev=3}+1, 0.01), to(-4, 0.1)}), to(0,0)}

-- aa = loop{ to((mutable(3)*0.9), 0.01)
--          , to(-4, 0.1)
--          }
aa = loop{ to((mutable{lev=3}+0.1)%3, 0.001)
         , to(-4, (mutable(0.001)+0.0001)%0.005 + 0.001)
         }
-- aa = loop{ to((dyn{lev=3}+2), 0.01)
--          , to(-4, 0.1)
--          }
-- output[1].action = aa
-- this compiles ok, but triggers a hardfault on execution
-- output[1]( asl._while(iterable(4), {to(4, 0.01), to(-4, 0.1)}))
-- output[1]( times(20, {to(4, 0.01), to(-4, 0.1)}))


-- TODO
-- enable math ops for dynamics (should just be overloading Dynmt with Itermt?)


-- TODO
-- slopes run at half speed for some reason?



function decompile(a, indent)
    indent = indent or 0
    if type(a) == 'number' then print(string.rep(' ', indent)..a)
    elseif type(a) == 'string' then print(string.rep(' ', indent)..a)
    elseif type(a) == 'function' then print(string.rep(' ', indent)..tostring(a))
    elseif type(a) == 'table' then
        local iters = 0
        for k,v in ipairs(a) do
            iters = iters + 1
            decompile(v, indent + 2)
        end
        if iters == 0 then print(string.rep(' ', indent),string.format("%s=%d",next(a))) end
        print(string.rep(' ', indent)..' ,')
    end
end


function init()
    decompile( aa )
    output[1].action = aa
    output[1]()
end
