--- scale quantizer generator library
-- call the library similarly to input.scale or output.scale
-- returns a function that will perform the quantization on an input

local Q = {}

Q.ji = function(rs, ins, outs)
    -- create a 12TET-ified version of rs
    return Q.__call(S, just12(rs), ins, outs)
end

local chrom = {0,1,2,3,4,5,6,7,8,9,10,11}
Q.__call = function(self, ns, ins, outs)
    -- defaults
    ins, outs = ins or 12, outs or 1
    -- chromatic shortcut with empty table or empty call
    ns = (not ns or #ns == 0) and chrom or ns

    -- optimize by precalculating constants
    local _INS = 1/ins -- inverse of in-scaling (mul cheaper than div!)
    local TET = 12 -- TODO configurable
    local _TET = 1/TET -- TODO configurable
    local OFF = 0.5 * ins / TET -- half an input-window of offset
    local LEN = #ns -- memoize length

    return function(n)
        local norm = (n+OFF) * _INS -- normalize to input scaling
        local octs = math.floor(norm) -- extract octaves
        local ix   = math.floor((norm - octs) * LEN)
        return (ns[ix+1]*_TET + octs) * outs -- scale lookup & reconstruct
    end
end

return setmetatable(Q,Q)
