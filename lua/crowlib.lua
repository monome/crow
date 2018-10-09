--- Crow standard library
--

local crow = {}

function crow.squared(n)
    return n*n
end



function rand( min, max )
    if type(min) == 'table' then
        local mn = min[1]
        local mx = min[2]
        return function() return rand( mn, mx ) end
    else return math.random( min, max ) end
end



print'crow lib loaded'

return crow
