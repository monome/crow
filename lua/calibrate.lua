local Cal = {}

function Cal.test()
    calibrate_now()
end

function Cal.default()
    calibrate_now(1)
end

function Cal.print()
    calibrate_print()
end

return Cal
