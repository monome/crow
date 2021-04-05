--- ASL in terms of sequins

-- sequins = require(sequins) -- 'sequins' is assumed to be a valid global for pointing to the sequins lib
S = sequins

local ASL = {}


--- global fns
function loop(t)
    return S(t) -- default sequins behaviour is looping
end




function once(t)
    return S(t):times(1) -- default sequins behaviour is looping
end


return ASL
