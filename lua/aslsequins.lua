--- ASL in terms of sequins

-- sequins = require(sequins) -- 'sequins' is assumed to be a valid global for pointing to the sequins lib
S = sequins

local ASL = { hold_state = false }

ASL.sethold = function(b)
    ASL.hold_state = b
end







--- global fns
function loop(t)
    return S(t) -- default sequins behaviour is looping
end

function times(n, t)
    return S(t):all():times(n)
end

function once(t)
    -- TODO refactor by adding a 'dead' terminal element in the table (rather than using these constructs)
    return S(t):all():times(1) -- default sequins behaviour is looping
end

local function is_held()
    return ASL.hold_state
end
function held(t)
    return S(t):condr(is_held)
end


return ASL
