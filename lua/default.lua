function init()
    -- start all the lfos
    for c=1, 4 do
        out[c].asl:action()
    end

    counter = Metro.alloc()
    counter.time = 0.001
    counter.count = -1
    counter.callback = count
    counter:start()
end

local position = 0
function count(c)
    position = position + 1
    print(collectgarbage('count'))
    print(position)
end
