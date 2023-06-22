-- cal rework
--
-- function read_avg(chan, reps)
--     reps = reps or 256
--     local v = 0.0
--     clock.sleep(0.01) -- latency & settling time
--     for i=1,reps do
--         v = v + input[chan].volts -- accumulate readings
--         clock.sleep(0.005)
--     end
--     return v / reps -- average
-- end
function stdev(t, mean)
	local collect = 0	
	for _,v in ipairs(t) do
		local diff = v-mean
		collect = collect + diff^2
	end
	return math.sqrt(collect / #t)
end

function avg(t)
	local sum = 0
	for _,v in ipairs(t) do
		sum = sum + v
	end
	return sum / #t
end

-- configuration:
-- repeats 	latency srate  100*stdev 	1000*stdev(total[#10])
-- 128 		0.05    0.01   0.15 		~0.12
-- 32 		0.05    0.04   0.15 		0.20 ~ 0.29
	-- sample-count is more important than measuring over time (duration 1.6s)
-- 512 		0.05    0.0025 0.15 		0.06
-- 1024 	0.05    0.001  0.15 		0.04
-- 256	 	0.05    0.001  0.15 		0.1 <<<< this is where i'm landing
-- 64	 	0.05    0.001  0.15 		0.15

-- 128 		0.02    0.005  0.15 		0.09
-- 32	    0.02 	0.005  0.15			0.20
-- 32	    0.02 	0.001  0.14 		0.27
-- 128	    0.02 	0.001  0.15			0.13

----- The above just proves that we should sample as quickly as possible (1ms interval)
-- and average as many times as we can bear
-- 256 is a good middle ground

-- real voltages are 2v4265 and -0v0186


-- settling time is only needed for 1ms (this is the shortest clock.sleep possible)
-- no settling time is observed when sampling a single sample!
------- FIXME must confirm that DAC settling time doesn't change this
------- the above is simply switching the MUX and measuring gnd/+2v5, so DAC not involved
-- input settling time is <1ms
-- output settling time is ~2.5ms
-- the two overlap, so we just use the longer (2.5)
-- add some wiggle room to be totally certain (4ms)

function read_avg(chan, reps)
    reps = reps or 256
    local vt = {}
    clock.sleep(0.004) -- input & output settling time (plus 50% margin)
    for i=1,reps do
        table.insert(vt, input[chan].volts) -- accumulate readings
        clock.sleep(0.001)
    end
    local mean = avg(vt)
    print(string.format("% 6.4f % 4.2f",mean,100*stdev(vt, mean)))
    return mean -- average
end

function get_input_scale(n)
    cal.source'gnd'
    -- clock.sleep(0.05)
    local gnd = read_avg(n)
    cal.source'2v5'
    -- clock.sleep(0.05)
    local vref = read_avg(n)
    return gnd, vref

    -- local new = 2.5/(zz-z)
    -- new = 0.5*(new-1) + 1
    -- cal.input[n].scale = new * cal.input[n].scale
    -- print('input scale  ',n,cal.input[n].scale,z,zz, new)
end

function get_output(n)
	output[n].volts = 0
	cal.source(n)
	local gnd = read_avg(1)
	output[n].volts = 2.5
	cal.source(n)
	local vref = read_avg(1)
	return gnd, vref
end


function input_scale(n)
	local gnd, vref = get_input_scale(n)
    local new_scale = 2.5/(vref-gnd)
    new_scale = 0.5*(new_scale-1) + 1 -- only step halfway there

    -- multiply against existing (scale to find adjustment)
    cal.input[n].scale = new_scale * cal.input[n].scale
    -- print('input scale  ',n,cal.input[n].scale,gnd,vref, new_scale)
end

function input_offset(n)
    clock.sleep(0.05)
    cal.source'gnd'
    local z = read_avg(n)
    cal.input[n].offset = cal.input[n].offset - z
    -- print('input offset ',n,cal.input[n].offset)
end


function output_scale(n)
    output[n].volts = 0
    cal.source(n)
    clock.sleep(0.05)
    local z = read_avg(1)
    output[n].volts = 2.5
    clock.sleep(0.05)
    local zz = read_avg(1)
    cal.output[n].scale = 2.5/(zz-z)
    print('output scale ',n,cal.output[n].scale,z,zz)
end

function output_offset(n)
    output[n].volts = 0
    clock.sleep(0.05)
    cal.source(n)
    local z = read_avg(1)
    cal.output[n].offset = -z
    print('output offset',n,cal.output[n].offset)
end

function clear_cal_settings()
    for n=1,2 do
        cal.input[n].offset = 0
        cal.input[n].scale = 1.0
    end
    for n=1,4 do
        cal.output[n].offset = 0
        cal.output[n].scale = 1.0
    end
end


function init()
    print('///////////////////////////')
    clock.run( function()
        clock.sleep(0.1) -- wait for repl to be ready
        print' '
        clear_cal_settings() -- clear everything before we start

        -- now we can run the calibration loop
        -- theoretically we get closer each time (?)
        local gnd_avg, vref_avg = {},{}
        for i=1,10 do
	        -- input_scale(1)
	        -- input_offset(1)
			-- local gnd, vref = get_input_scale(1)
			-- can reach 0.01% scale error reliably
			-- print("scale error%", 100*((2.5/(vref-gnd))-1))
			-- print("offset error%", gnd*100)

			-- local gnd, vref = get_input_scale(1)
			local gnd, vref = get_output(1)
	        table.insert(gnd_avg, gnd)
	        table.insert(vref_avg, vref)


	        -- cal.input[2].scale = cal.input[1].scale
	        -- input_offset(2)
	        -- for n=1,4 do
	        --     output_scale(n)
	        --     output_offset(n)
	        -- everythingd
	    end
	    print(string.format("\n>>  % 6.4f % 6.4f",avg(gnd_avg),1000*stdev(gnd_avg, avg(gnd_avg))))
	    print(string.format("\n>>  % 6.4f % 6.4f",avg(vref_avg),1000*stdev(vref_avg, avg(vref_avg))))

        -- cal.save()
    end)
end

-- -- extended version with statistics info
-- function read_avg(chan, reps)
--     reps = reps or 256
--     local vt = {}
--     clock.sleep(0.01) -- latency & settling time
--     for i=1,reps do
--         table.insert(vt, input[chan].volts) -- accumulate readings
--         clock.sleep(0.005)
--     end
--     local mean = avg(vt)
--     print(string.format("% 6.4f % 6.4f % 6.4f % 6.4f",mean,stdev(vt, mean), min(vt), max(vt)))
--     return mean -- average
-- end
