--- simple calibration helpers
function offseter(chan, vol, coeff)
    local off = cal.input[chan].offset
    local exact = off-vol
    cal.input[chan].offset = off + (-vol*coeff)
end

function scalar(vol, coeff)
    local skal = cal.input[1].scale
    local exact = (skal * 2.5) / vol
    cal.input[1].scale = skal + coeff * (exact-skal) -- linear interpolate
end

function await_read(chan)
    local REPS = 32
    local v = 0.0
    for i=1,REPS do
        clock.sleep(0.002) -- sleep first for settling time
        v = v + input[chan].volts -- accumulate readings
    end
    return v / REPS -- average
end

function cal_input1(coeff, iter)
    cal.source'gnd'
    offseter(1, await_read(1), coeff)
    local oerr = -input[1].volts
    cal.source'2v5'
    scalar(await_read(1), coeff)
    -- print mV error at 0V and 2V5
    print(string.format("%i %.3f %.3f", iter, 1000*oerr, 1000*(input[1].volts-2.5)))
end

function cal_input2(coeff, iter)
    cal.source'gnd'
    offseter(2, await_read(2), coeff)
    print(string.format("%i %.3f", iter, -1000*input[2].volts))
end

function clock_c()
    clock.run(function()
        -- this routine reliably gets under 1mV error at both 0V & 2V5
        for i=1,6 do cal_input1(0.75, i) end
        for i=1,6 do cal_input1(0.25, i) end

        cal.input[2].scale = cal.input[1].scale -- copy scale as we can't test input 2 scaling
        for i=1,6 do cal_input2(0.75, i) end
        for i=1,6 do cal_input2(0.25, i) end

        -- for i=1,64 do
        --     cal.source'gnd'
        --     offseter(await_read(1))
        --     -- print(i, 'expect 0.0',input[1].volts)
        --     local oerr = -input[1].volts
        --     cal.source'2v5'
        --     scalar(await_read(1))
        --     -- print(i,'expect 2.5',input[1].volts)
        --     -- print(i, 1000*oerr, 1000*(input[1].volts-2.5))
        --     print(string.format("%i %.3f %.3f", i, 1000*oerr, 1000*(input[1].volts-2.5)))
        -- end
        cal.source'gnd'
        -- clock.sleep(0.01)
        -- print('expect 0.0',input[1].volts)
    end)
end
