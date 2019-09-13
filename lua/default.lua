function init()
    -- start all the lfos
    for c=1, 4 do
        output[c].asl:action()
    end

    ii.txi.get( 'value', 1 )
    --input[1].mode('change', 0.5, 0.1, 'both')
end

-- FIXME need to access 'ii.<mod>' before defining event bc it gets trampled
-- by the setup otherwise? i don't understand why the table access doesn't cause
-- the library to be loaded, have default event, then apply the .event here to
-- overwrite it?
--local ignore = ii.txi.get
--ii.txi.event = function( e, data )
--    if e == 'value' then debug_usart('value='..data)
--    end
--end

--local position = 0
--function count(c)
--    position = position + 1
--    print(position)
--end
