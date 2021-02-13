casl = dofile("lua/casl.lua")

-- helper functions for debugging
tab={}
tab.print = function(t)
  for k,v in pairs(t) do print(k .. '\t' .. tostring(v)) end
end

tab.dump = function(t,indent,name)
    indent = indent or 0
    local si = string.rep(' ',indent*2)
    local s = si
    if type(name)=='string' then s = s .. name..'=' end
    s = s .. '{ '
    for k,v in pairs(t) do
        if( type(v) == 'table') then
            s = s .. '\n' .. tab.dump(v,indent+1,k) .. '\n' .. si
        else
            if type(k) == 'string' then
                s = s .. k .. '='
            end
            s = s .. tostring(v) .. ', '
        end
    end
    return s .. '}'
end

function casl_describe(id, d)
    print('describe['..id..']='..tostring(d))
    print(tab.dump(d))
end

dyns = {}
dyn_ix = 0
function casl_defdynamic(id)
    local d = dyn_ix
    print('defdynamic['..id..']='..d)
    dyn_ix = dyn_ix + 1
    return d
end

function casl_setdynamic(id, ix, val)
    print('set['..ix..']='..val)
    dyns[ix] = val
end


a = casl.new()
-- a:describe( dyn{level=3.0} )
-- a:describe( cto(3.0,4.2) )
-- a:describe( cto(3.0,4.2,'linear') )
-- a:describe{ cto(-3,3,'linear'), cto(3,3,'linear') }
-- a:describe(cloop{ cto(-3,3,'linear') })
-- a:describe(cloop{ cto(cdyn{volts=2.1},3,'linear') })
a:describe(cloop{ cto(cdyn{volts=2.1},cdyn{tim=3},'linear') })
casl.dyn.volts(3)
casl.dyn.tim(0.1)
-- a:describe( to(dyn{le=3.0},4.2) )





--- C-ASL library
--
-- a collection of C-ASL descriptors, converted from asllib

-- NB: these lib functions would have default vals like asllib

-- --- LFO
-- function lfo_literal( time, level, shape )
--     return loop{ to(       level , div( time, 2), shape )
--                , to( sub(0,level), div( time, 2), shape )
--                }
-- end

-- -- using dynamics (same dynamic can be shared in multiple places)
-- function lfo_dynamic( time, level, shape )
--     -- caution here: dyn{level=level} means
--         -- register a dynamic with the name "level"
--         -- and set it's initial value to the argument level
--     return loop{ to( dyn{level=level}
--                    , div( dyn{time=time}, 2)
--                    , dyn{shape=shape} )
--                , to( sub(0,dyn{level=level})
--                    , div( dyn{time=time}, 2)
--                    , dyn{shape=shape} )
--                }
-- end

--- usage
-- literal
-- cc = casl.new()
-- cc.action = lfo_literal()   -- assign the casl action
-- cc.action()                 -- tell the asl to start running
-- cc.dyn.level(2.0)           -- ERROR. there is no dynamic in lfo_literal()

-- -- dynamic
-- ccc = casl.new()
-- ccc.action = lfo_dynamic()   -- assign the casl action
-- ccc.action()                 -- tell the asl to start running
-- ccc.dyn.level(2.0)           -- updates 'level' dynamic to new value


-- --- because we're in Lua, and really just passing around functions & tables, we can abstract the core to() calls from dyn{}
-- d = casl.new()
-- d.action = lfo_literal( dyn{time=1.0}
--                       , dyn{volume=5.0} -- note we can use whatever name we want
--                       , dyn{shape='sine'}
--                       )
-- d.action()
-- d.dyn.volume(3.3) -- works even though it's the 'literal' form of lfo

