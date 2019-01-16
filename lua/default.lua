function init()
    -- start all the lfos
    for c=1, 4 do
        out[c].asl:action()
    end

    metro = Metro.assign_all()

    --input[1].mode('change', 0.5, 0.1, 'both')
end

local position = 0
function count(c)
    position = position + 1
    print(position)
end
