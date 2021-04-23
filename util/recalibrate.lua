--- recalibrate
-- perform a recalibration of crow's hardware i/o
-- begin with calibrate()

VERBOSE = false -- true will print mV error at each step

function pprint()
    local function pprintset(key)
        print(key..'s: ')
        for k,v in ipairs(cal[key]) do
            print(string.format('  %i  offset: % 01.4f,  scale:% 01.4f', k, v.offset, v.scale))
        end
    end
    print' offset should be near 0'
    print' scale should be near 1'
    pprintset'input'
    pprintset'output'
end

function validate(elem)
    local function val(elem, key, min, max)
        local v = elem[key]
        if v < min or v > max then
            print(elem.chan, 'invalid '..key)
            return true
        end
    end
    local e = val(elem, 'offset', -0.2, 0.2)
    return e or val(elem, 'scale', 0.8, 1.2)
end

function validate_all()
    local e = false
    for i=1,2 do e = e or validate(cal.input[i]) end
    for i=1,4 do e = e or validate(cal.output[i]) end
    return e
end

-- reads input a number of times and returns the average value
-- MUST be called from inside a clock
function read_avg(chan, reps)
    reps = reps or 32
    local v = 0.0
    clock.sleep(0.005) -- latency & settling time
    for i=1,reps do
        v = v + input[chan].volts -- accumulate readings
        clock.sleep(0.002)
    end
    return v / reps -- average
end


function step_offset(chan, vol, coeff)
    chan.offset = chan.offset + (-vol*coeff)
end

function step_scale(chan, vol, coeff)
    local skal = chan.scale
    local exact = (skal * 2.5) / vol
    chan.scale = skal + coeff * (exact-skal) -- linear interpolate
end

function cal_input1(coeff)
    -- read both states
    cal.source'gnd'
    local rr = read_avg(1)
    cal.source'2v5'
    local rrr = read_avg(1)

    -- step toward ideal values
    step_offset(cal.input[1], rr, coeff)
    step_scale(cal.input[1], rrr-rr, coeff)

    -- read new result (refactor so this becomes the new input?)
    cal.source'gnd'
    rr = read_avg(1, 4)
    cal.source'2v5'
    rrr = read_avg(1, 4)
    if VERBOSE then print(string.format("input[1] %.3f %.3f", -1000*rr, 1000*(rrr-2.5))) end
    return -rr, rrr-2.5 -- return error levels
end

function cal_input2(coeff)
    cal.source'gnd'
    local rr = read_avg(2)

    step_offset(cal.input[2], rr, coeff)

    cal.source'gnd'
    rr = read_avg(1, 4)
    if VERBOSE then print(string.format("input[2] %.3f", -1000*input[2].volts)) end
    return -rr -- return error
end

-- exclusive out: set chan to volts, and all others to zero
function xout(chan, volts)
    for i=1,4 do output[i].volts = (i==chan) and volts or 0 end
end

function cal_output(chan, coeff)
    cal.source(chan)

    -- read value at 0 & 2v5 before making changes
    xout(chan, 0)
    local rr = read_avg(1)
    xout(chan, 2.5)
    local rrr = read_avg(1)

    step_offset(cal.output[chan], rr, coeff)
    step_scale(cal.output[chan], rrr-rr, coeff) -- send *difference* from 'zero'

    xout(chan, 0)
    rr = read_avg(1, 4)
    xout(chan, 2.5)
    rrr = read_avg(1, 4)

    xout(-1, 0) -- set all chans to 0
    if VERBOSE then print(string.format("output[%i] %.3f %.3f",chan, -1000*rr, 1000*(rrr-2.5))) end
    return -rr, rrr-2.5 -- return error levels
end

-- return true if all args are within range around zero
function in_window(window, ...)
    local args = {...}
    for k,v in pairs(args) do
        if v < -window or v > window then return nil end -- return on first failure
    end
    return true -- all succeeded
end
-- returns an in_window fn with window arg partially applied
function make_win_checker(window) return function(...) return in_window(window, ...) end end

-- executes 2nd arg with any varargs, then tests (multiple) results with pred fn
function do_until(pred, fn, ...)
    if not pred(fn(...)) then do_until(pred, fn, ...) end -- recur until pred() succeeds
end

function init()
    print ' '
    print '1. remove all jacks from crow'
    print '2. run calibrate()'
    print '     if calibration succeeds, it will be saved to flash memory'
    print '3. view the calibration values with pprint()'
end

function calibrate()
    print('calibrating...')
    clock.run(function()
        local is_wide_win = make_win_checker(0.001) -- 1mV
        local is_tight_win = make_win_checker(0.0002) -- 200uV

        print('  input[1]')
        do_until(is_wide_win, cal_input1, 0.75)
        do_until(is_tight_win, cal_input1, 0.15)

        print('  input[2]')
        cal.input[2].scale = cal.input[1].scale -- copy scale as we can't test input 2 scaling
        do_until(is_wide_win, cal_input2, 0.75)
        do_until(is_tight_win, cal_input2, 0.15)

        for n=1,4 do
            print('  output['..n..']')
            do_until(is_wide_win, cal_output, n, 0.75)
            do_until(is_tight_win, cal_output, n, 0.15)
        end

        cal.source'gnd' -- ensure input[1] normalled to ground

        if validate_all() then
            print('calibration failed:')
            pprint()
        else
            print('validated. saving...')
            cal.save()
        end
    end)
end
