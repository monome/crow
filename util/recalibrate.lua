--- recalibrate
-- performs a recalibration of crow's hardware i/o

function pprint()
    local function pprintset(key)
        print(key..'s: ')
        for k,v in ipairs(cal[key]) do
            print('', k, 'offset:' .. v.offset, 'scale:' .. v.scale)
        end
    end
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

function await_read(chan)
    local v = 0.0
    for i=1,32 do
        v = v + input[chan].volts
        clock.sleep(0.002)
    end
    return v / 32
end

-- inc & mul will be used when we iteratively find optimum levels
function set(chan, param, level) chan.param = level end
function inc(chan, param, amount) chan.param = chan.param + amount end
function mul(chan, param, scale) chan.param = chan.param * scale end

-- set a single chan's voltage, and all others to zero
function xout(chan, volts)
    for i=1,4 do output[i].volts = (i==chan) and volts or 0 end
end

function init()
    clock.run(recalibrate)
end

VREF = 2.5
function recalibrate()
    --- ADC calibration
    cal.source'gnd'
    set(cal.input[1], 'offset', -await_read(1))

    cal.source'2v5'
    local v = VREF/(await_read(1) - cal.input[1].offset)
    set(cal.input[1], 'scale', v)
    set(cal.input[2], 'scale', v) -- copy to input[2] as no dedicated hw

    cal.source'gnd'
    inc(cal.input[1], 'offset', -await_read(1))
    inc(cal.input[2], 'offset', -await_read(2))

    --- DAC calibration
    for i=1,4 do
        cal.source(i)
        xout(i, 0)
        set(cal.output[i], 'offset', -await_read(1))
        xout(i, VREF)
        set(cal.output[i], 'scale', await_read(1))
        -- redo offset after updating scale
        xout(i, 0)
        set(cal.output[i], 'offset', -await_read(1))
    end
    xout(0, 0) -- all off

    if validate_all() then print'error validation failed'
    else cal.save() end
end
