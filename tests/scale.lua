--- scale library test

--- Just Intonation helpers
-- convert a single fraction, or table of fractions to just intonation
-- optional 'offset' is itself a just ratio
-- justvolts converts to volts-per-octave
-- just12 converts to 12TET representation (for *.scale libs)
-- just12 will convert a fraction or table of fractions into 12tet 'semitones'
function _justint(fn, f, off)
    off = off and fn(off) or 0 -- optional offset is a just ratio
    if type(f) == 'table' then
        local t = {}
        for k,v in ipairs(f) do
            t[k] = fn(v) + off
        end
        return t
    else -- assume number
        return fn(f) + off
    end
end
JIVOLT = 1 / math.log(2)
JI12TET = 12 * JIVOLT
function _jiv(f) return math.log(f) * JIVOLT end
function _ji12(f) return math.log(f) * JI12TET end
-- public functions
function justvolts(f, off) return _justint(_jiv, f, off) end
function just12(f, off) return _justint(_ji12, f, off) end
function hztovolts(hz, ref)
    ref = ref or 261.63 -- optional. defaults to middle-C
    return justvolts(hz/ref)
end

qscale = dofile("../lua/scale.lua")


--- major scale quantizer
-- input[1].mode('scale',{0,2,4,7,9},12,1.0)
-- output[1].scale({0,2,4,7,9},12,1.0)

-- myquantizer = scale({0,2,4,7,9}, 12, 1.0) -- convert to volts
-- myquantizerN = scale({0,2,4,7,9}, 12, 12) -- input/output mapping the same

-- think of the 'divs'/'scaling' as input/output ranges
-- so if divs == 12, then our table should be in 12TET
-- if divs == 'ji', then our table should be in just fractions
-- if scaling == divs, then output matches input
-- if divs==12 & scaling==1 then convert note table to voltage
--[[
-- quantize to octaves, staying in 12TET
local s1 = qscale({0},12,12)
assert(type(s1) == 'function')
assert(s1(0) == 0)
assert(s1(7) == 0)
assert(s1(11) == 0) -- always round down
assert(s1(11.99) == 12) -- capture marginally beneath bounds
assert(s1(12) == 12)
assert(s1(-12) == -12) -- test negative numbers
assert(s1(-11) == -12)
assert(s1(-13) == -24)

-- quantize to octaves, convert to volts
local s1 = qscale({0},12,1)
assert(s1(0) == 0)
assert(s1(7) == 0)
assert(s1(11) == 0) -- always round down
assert(s1(11.99) == 1) -- capture marginally beneath bounds
assert(s1(12) == 1)
assert(s1(-12) == -1) -- test negative numbers
assert(s1(-11) == -1)
assert(s1(-13) == -2)

-- quantize to 2 values, splitting the octave evenly
local s1 = qscale({0,1},12,12)
assert(s1(0) == 0)
assert(s1(5) == 0) -- round down
assert(s1(6) == 1) -- round up
assert(s1(12) == 12) -- round up

-- as above but confirm default in/out is 12,1
local s1 = qscale({0,36})
assert(s1(0) == 0)
assert(s1(5) == 0) -- round down
assert(s1(6) == 3) -- round up (note out-of-octave vals allowed)
assert(s1(12) == 1) -- round up

-- chromatic n->v scaler
local s1 = qscale()
assert(s1(0) == 0)
assert(s1(1) == 1/12)
assert(s1(11) == 11/12)
assert(s1(12) == 1)

-- chromatic n->v scaler: table-call syntax
local s1 = qscale{}
assert(s1(0) == 0)
assert(s1(1) == 1/12)
assert(s1(11) == 11/12)
assert(s1(12) == 1)

-- chromatic n->n scaler
local s1 = qscale({}, 12, 12)
assert(s1(0) == 0)
assert(s1(1) == 1)
assert(s1(11) == 11)
assert(s1(12) == 12)

-- just intonation support
-- use a separate function
local sj1 = qscale.ji({1/1, 2/1}, 12, 1)
assert(sj1(0) == 0)
assert(sj1(5) == 0) -- round down
-- note we need to account for floating point inaccuracies here
assert(sj1(6) >= 0.9999 and sj1(6) <= 1.0001) -- round up
assert(sj1(12) == 1.0) -- round up
]]
-- input as voltage
local sv1 = qscale({0,7},1.0,12)
print(sv1(0))
assert(sv1(0) == 0)
assert(sv1(0.5) == 7)
