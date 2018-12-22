_t = dofile("util/test.lua")
crow = dofile("lua/crowlib.lua")

--- setup mocks

--- Tests
function run_tests()

    --_t.run( luapath_to_cpath
    --      , {'lua/asl.lua' , 'lua_asl' }
    --      )

    -- remote callback
    -- FIXME can't test bc they print directly rather than through a lib fn..
    io_get_input = function(channel) return 6.6 end
    --_t.run( get_cv
    --      , {1 , '^^ret_cv(1,6.6)' }
    --      )

    --_t.type( {{Asl.new()}   , 'table' } -- should cause warning

    -- test standard calls
    _t.run( math.max
        , {{3, 4}, 4}
        )
    -- test closured (curlies deferred) calls
    --_t.run( function(table)
    --            local closure = math.max(table)
    --            print(type(closure))
    --            print(closure[1])
    --            return closure()
    --        end
    --    , {{{3, 4}}, 4}
    --    )

    function do_it(message) print(message) end
    do_it = closure_if_table( do_it )
    do_it('now')
    do_it{'later'}()

end

run_tests()
