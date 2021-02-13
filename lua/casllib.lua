--- C-ASL library
--
-- a collection of C-ASL descriptors, converted from asllib

-- NB: these lib functions would have default vals like asllib

--- LFO
-- function lfo_literal( time, level, shape )
--     return loop{ to(       level , div( time, 2), shape )
--                , to( sub(0,level), div( time, 2), shape )
--                }
-- end

-- using dynamics (same dynamic can be shared in multiple places)
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

print'todo casllib'
