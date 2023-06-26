--- cali
-- utility for calibrating crow's CV i/o
--
-- keep druid open while running!
-- 
-- when the script starts, it will run the calibration automatically
-- input[1] and all outputs will be calibrated automatically
-- then you'll be prompted to add a patch cable
--    connect output[1] to input[2]
-- now the calibration will complete
-- if ALWAYS_TEST is true, you'll be prompted to remove the patch cable
--    test results will appear gradually
--
-- re-run the calibration with: calibrate()
-- run the test-suite with: test()
-- print the raw offsets & scalars with: raw()


-- settings
local AUTOSTART   = true  -- if false, you can calibrate by typing: calibrate()
local ALWAYS_TEST = true -- if true, the test() function will autorun after calibrate()

-- debug: use when working on this script
local AUTOSAVE    = true  -- set false while debugging to reduce flash write cycles
local ALWAYS_RAW  = false -- set true to print raw offset & scale values after calibrate()
local ITERATIONS  = 1     -- set higher than 1 to show that you have found the *correct* solution


--- these functions are generally unused
-- values seem to be accurate enough by simply measuring & calculating directly
-- no need to measure, update, re-measure (etc)

function lerp(from, to, coeff)
    return from + coeff*(to - from)
end

-- diff_multiplier is 100 ~ 2000 or so. sets convergence rate / sensitivity
function adaptive_filter(past, new, diff_multiplier)
    local coeff = math.min(1, diff_multiplier * math.abs(past - new))
    local dest = lerp(past, new, coeff)
    return dest
end


-- specifically tuned for minimal standard deviation across multiple calls
-- see: util/adda-measurement.lua for methodology
-- settling time increased for out[3]. limit ~12ms. 20ms for safety
function read_avg(chan)
    local reps = 256
    local sum = 0
    clock.sleep(0.02) -- input & output settling time
    for i=1,reps do
        sum = sum + input[chan].volts -- accumulate readings
        clock.sleep(0.001)
    end
    return sum / reps -- average
end

function xvolts(ch, v)
    for n=1,4 do
        output[n].volts = n==ch and v or 0
    end
end

function sample(t)
    if t.volts then xvolts(t.source, t.volts) end -- set source voltage exclusively
    cal.source(t.source) -- select source
    return read_avg(t.input)
end

function input_scale(n)
    local gnd, vref
    if n==1 then
        gnd  = sample{input=n, source='gnd'}
        vref = sample{input=n, source='2v5'}
    else -- n==2
        gnd  = sample{input=n, source=1, volts=0}
        vref = sample{input=n, source=1, volts=2.5}
    end

    local past = cal.input[n].scale -- capture past scalar
    local new = 2.5/(vref-gnd)      -- determine new scalar
    new = new * past                -- account for past scalar

    -- move toward solution
    cal.input[n].scale = adaptive_filter(past, new, 1000)

    print(string.format('input[%i].scale  = % 6.5f (% 6.5f, % 6.5f)',n,cal.input[n].scale,gnd,vref))
end

function input_offset(n)
    local gnd
    if n==1 then
        gnd = sample{input=n, source='gnd'}
    else -- n==2
        gnd = sample{input=n, source=1, volts=0}
    end

    local past = cal.input[n].offset -- capture past offset
    local new = gnd                  -- determine new offset
    new = past - new                 -- account for past offset

    -- move toward solution
    cal.input[n].offset = adaptive_filter(past, new, 300)

    print(string.format('input[%i].offset = % 6.5f (% 6.5f)',n,cal.input[n].offset,gnd))
end

function output_scale(n)
    local gnd  = sample{input=1, source=n, volts=0}
    local vref = sample{input=1, source=n, volts=2.5}

    local past = cal.output[n].scale -- capture past scalar
    local new = 2.5/(vref-gnd)       -- determine new scalar
    new = new * past                 -- account for past scalar

    -- move toward solution
    cal.output[n].scale = adaptive_filter(past, new, 1000)

    print(string.format('output[%i].scale  = % 6.5f (% 6.5f, % 6.5f)',n,cal.output[n].scale,gnd,vref))
end

function output_offset(n)
    local gnd = sample{input=1, source=n, volts=0}

    local past = cal.output[n].offset -- capture past offset
    local new = gnd                   -- determine new offset
    new = new / cal.output[n].scale   -- account for scale being applied already
    new = past - new                  -- account for past offset

    -- move toward solution
    cal.output[n].offset = adaptive_filter(past, new, 300)

    print(string.format('output[%i].offset = % 6.5f (% 6.5f)',n,cal.output[n].offset,gnd))
end

function await_input(n)
    if sample{input=n, source=1, volts=-3} > -2 then
        if n == 1 then print '\ndisconnect all patch cables'
        else print '\nconnect output[1] -> input[2]' end

        while sample{input=n, source=1, volts=-3} > -2 do
            tell('stream',n,input[n].volts)
            clock.sleep(0.1)
        end
        print'ok!'
        clock.sleep(1) -- wait 1 second to ensure both ends of cable are disconnected
    end
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

function find_error_in_cents(result, expected)
    return math.floor(0.5 + 1200*(result - expected))
end

function test()
    clock.run(function()
        -- ensure there's no cable remaining from calibration
        await_input(1)

        local pass = true -- assume true unless we see an error
        local failmsgs = {}
        -- 30, 3, 5, 12, 30

        print'\nvoltage listed to the nearest millivolt'
        print'error amounts listed in cents beneath the readings'
        print'  expect:   -2.5    0.0    2.5    5.0    7.5\n'

        -- input 1
        local gnd  = sample{input=1, source='gnd'}
        local vref = sample{input=1, source='2v5'}
        local gnd_err  = find_error_in_cents(gnd, 0)
        local vref_err = find_error_in_cents(vref, 2.5)
        print(string.format('input[1]           % 01.3f % 01.3f',gnd, vref))
        print(string.format('              % 7d % 7d',gnd_err,vref_err))
        if math.abs(gnd_err) > 3 then pass = false end
        if math.abs(vref_err) > 3 then pass = false end

        -- input 2
        gnd = sample{input=2, source='gnd'}
        gnd_err = find_error_in_cents(gnd, 0)
        print(string.format('input[2]           % 01.3f', gnd))
        print(string.format('              % 7d',gnd_err))
        if math.abs(gnd_err) > 3 then pass = false end

        for n=1,4 do
            local V = {-2.5, 0, 2.5,  5, 7.5} -- test voltages
            local E = {  30, 3,   5, 12,  30} -- allowable cents error at each voltage
            local t = {} -- test results
            local e = {} -- error calculation in cents
            for k,v in ipairs(V) do
                t[k] = sample{input=1, source=n, volts = v}
                e[k] = find_error_in_cents(t[k], v)
                if math.abs(e[k]) > E[k] then
                    pass = false
                    table.insert(failmsgs, 'output['..n..'] @'..v..'V out of range')
                end
            end
            print(string.format('output[%i]   % 01.3f % 01.3f % 01.3f % 01.3f % 01.3f',n
                 ,t[1],t[2],t[3],t[4],t[5]))
            print(string.format('       % 7d % 7d % 7d % 7d % 7d'
                 ,e[1],e[2],e[3],e[4],e[5]))
        end
        if pass then
            print('\n---- PASS ----')
        else
            print('\n!!!! FAIL !!!!')
            print(table.concat(failmsgs, ", "))
        end
    end)
end

function raw()
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


function calibrate()
    clock.run( function()
        clear_cal_settings() -- clear everything before we start

        -- input 1
        await_input(1)
        for i=1,ITERATIONS do
            input_scale(1)
            input_offset(1)
        end

        -- outputs
        for i=1,ITERATIONS do
            for n=1,4 do
                output_scale(n)
                output_offset(n)
            end
        end

        -- input 2
        await_input(2)
        for i=1,ITERATIONS do
            input_scale(2)
            input_offset(2)
        end

        if AUTOSAVE    then cal.save() end
        if ALWAYS_RAW  then raw() end
        if ALWAYS_TEST then test() end
    end)
end

function init()
    print('///////////////////////////')
    if AUTOSTART then calibrate() end
end
