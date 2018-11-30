-- example crow script
--
-- demonstrating how to use the hardware IO
-- uses timers for delay
-- demonstrates slew engine & lfos
--
-- in.1  continuous CV input
-- in.2  threshold trigger input
-- out.1 echo in.1 with slew
-- out.2 echo in.1, quantized
-- out.3 lfo, in.1 = speed, in.2 = retrig
-- out.4 echo in.2 with delay set by in.1

function hz( seconds ) return 1/seconds end

function syntax()
    o[1]:action = lfo( 2, 'expo' ) -- set the action
    o[1]:action()                  -- do the action. optional bool/arg?



end


-- most of these param setups should be *defaults*
function init()
    -- configure input 1
    in.rate(1, MAX)

    -- configure input 2 as schmitt trigger
    in.event(2, RISE) -- trigger on rising edge
    in.thresh(2, 4.0) -- trigger around 4 volts
    in.hyst(2, 1.0)   -- 1 volt hysteresis (rise@5v, fall@3v)

    -- setup scale for quantizer
    quantize.mode("volts")           -- converts 1v/octave
    quantize.base(0.0)               -- 0v is root
    quantize.scale{ 0,2,4,5,7,9,11 } -- can be any length!

    -- setup LFO
    out.set(3, 0.0)
    out.level(3, 10.0)
    out.rate(3, 1.0)
    out.shape(3, SINE)

    -- configure out 4 as trigger pulse
    out.trig.pos(4, RISE)
    out.trig.time(4, 0.01)  -- 10 milliseconds
    out.trig.level(4, 10.0) -- 10v

    -- allocate a metro for trigger delay
    delay = metro.new( tdelay, 1.0, 1 )
end

function tdelay()
    out.trig(4)
end

-- callback when an input channel crosses a threshold
-- n is which channel [1,2]
-- d is direction. boolean where true = rising
function trig(n,d)
    if n == 2 and d == true then -- already guaranteed by in[2].event setup
        out.reset(3)
        delay.time = in.value(1) -- sample the cv at in.1
        delay:start()            -- set delay retrigger
    end
end

function stream(n,value)
    if n == 1 then
        out.set(1, value, 0.1)      -- follow input with slew=0.1
        out.set(2, quantize(value)) -- follow input quantized
        out.rate(3, vtohz(value))
    end
end
