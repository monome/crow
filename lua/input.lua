--- cv input library
-- examples of how the user will interract with the cv input

in = {1,2} -- global table

function eg_init()
    local current_value = in[1].value

    in[1].mode('none')
    in[1].mode('stream')
    in[1].mode('change')

    in[1]{ mode       = 'stream'
         , time       = 0.1
         }

    -- following two are equivalent
    in[1]{ mode       = 'change'
         , threshold  = 0.5
         , hysteresis = 0.1
         , direction  = 'rising'
         }
    in[1].mode('change',0.5,0.1,'rising')

    -- future modes which require different c handling & diff lua callback hooks
    in[1].mode('window',{},0.1,'both') -- where table is the list of voltages
    in[1].mode('scale',{0,2,4,6,7,9,11})
    in[1].mode('quantize',19,{0,4,7,12})
    in[1].mode('ji',{1/1,3/2,4/3})
end

function in[1].stream(value)
    --todo
end

function in[1].change(value)
    --
end
