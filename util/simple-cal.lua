-- cal rework
--
function read_avg(chan, reps)
    reps = reps or 32
    local v = 0.0
    clock.sleep(0.01) -- latency & settling time
    for i=1,reps do
        v = v + input[chan].volts -- accumulate readings
        clock.sleep(0.005)
    end
    return v / reps -- average
end

function input_scale(n)
	cal.input[n].offset = 0
	cal.input[n].scale = 1.0
	cal.source'gnd'
	clock.sleep(0.05)
	local z = read_avg(n)
	cal.source'2v5'
	clock.sleep(0.05)
	local zz = read_avg(n)
	cal.input[n].scale = 2.5/(zz-z)
	print('input scale',n,cal.input[n].scale,z,zz)
end

function input_offset(n)
	cal.input[n].offset = 0
	clock.sleep(0.05)
	cal.source'gnd'
	local z = read_avg(n)
	cal.input[n].offset = -z
	print('input offset',n,cal.input[n].offset)
end

function output_scale(n)
	cal.output[n].offset = 0
	cal.output[n].scale = 1.0
	output[n].volts = 0
	cal.source(n)
	clock.sleep(0.05)
	local z = read_avg(1)
	output[n].volts = 2.5
	clock.sleep(0.05)
	local zz = read_avg(1)
	cal.output[n].scale = 2.5/(zz-z)
	print('output scale',n,cal.output[n].scale,z,zz)
end

function output_offset(n)
	cal.output[n].offset = 0
	output[n].volts = 0
	clock.sleep(0.05)
	cal.source(n)
	local z = read_avg(1)
	cal.output[n].offset = -z
	print('output offset',n,cal.output[n].offset)
end



function init()
	print('///////////////////////////')
	clock.run( function()
		input_scale(1)
		input_offset(1)
		cal.input[2].scale = cal.input[1].scale
		input_offset(2)
		for n=1,4 do
			output_scale(n)
			output_offset(n)
		end

		cal.save()
		--[[
		input[1].mode('stream')
		output[1].volts = 0
		output[1].volts = 1
		output[1].volts = 2
		output[1].volts = 4
		output[1].volts = 8
		output[1].volts =-2 
		]]--
	end)
end


