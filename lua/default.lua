function init()
    out[1].asl:bang(true)
--    go_toward( 1, 5, 10, 'linear' )
--
--    go_toward{ channel  = 1
--             , level    = 5
--             , time     = 10
--             , slope    = 'linear'
--             }

--    slope[1]:action( lfo() )
--    slope[1]:bang(true)
--
--    -- ideally (with metatables)
--    out[1].action = lfo()   -- set action to be lfo (must clear 'locked')
--    out[1]:action(true)     -- do action with 'high' state (ie. reset)
end
