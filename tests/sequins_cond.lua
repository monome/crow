--- sequins.lua tester


s = dofile("lua/sequins_cond.lua")

--- make a table of notes, with default next() behaviour
local s1 = s{0,4,7,11}
assert(s1() == 0)
assert(s1() == 4)
assert(s1() == 7)
assert(s1() == 11)
assert(s1() == 0) -- wraps

-- custom behaviour: steps backward by 1
-- local s2 = s({0,3,6,9}, s.step(-1))
local s2 = s.new{0,3,6,9}:step(-1)
assert(s2() == 0)
assert(s2() == 9)
assert(s2() == 6)
assert(s2() == 3)

--- default behaviour, demonstrate select for direct setting
-- local s3 = s{1,2,3,4,5}
-- -- for i=1,5 do print(s2()) end
-- assert(s3() == 1)
-- assert(s3() == 2)
-- assert(s3:select(4)() == 4) -- select() is 1-based like the rest of lua
-- assert(s3() == 5)
-- assert(s3() == 1)

--[[
--- explicit behaviours (any datatype!)
-- local s4 = s{'a','b','c','d'}
-- assert(s4:select(1) == 'a')
-- assert(s4:select(1) == 'a')
-- assert(s4:select(3) == 'c')
-- assert(s4:step(0) == 'c')
-- assert(s4:step(2) == 'a') -- wraps
-- assert(s4:step(2) == 'c')
-- assert(s4:step(-1) == 'b')
--- custom next fn (use this to make rand / drunk options)
-- local s6 = s({1,2,3}, function(self) return self:step(1) end)
-- assert(s6() == 1)
-- assert(s6() == 2)
-- assert(s6() == 3)

]]




--- nesting sequins
local s7 = s{1, s{2, s{3, 4}}}
assert(s7() == 1)
assert(s7() == 2)
assert(s7() == 1) -- Note how it interleaves nested tables. ie. outer sequence has precedence
assert(s7() == 3)
assert(s7() == 1)
assert(s7() == 2)
assert(s7() == 1)
assert(s7() == 4)

--- behaviours can take sequins as args
-- this is really weird, but it makes it such that sequins become first class value/object dualities
local s12 = s{1,2,3}:step(s{1,1,-1}) -- 2 steps forward, 1 step back
assert(s12() == 1)
assert(s12() == 2)
assert(s12() == 3)
assert(s12() == 2)
assert(s12() == 3)
assert(s12() == 1)
assert(s12() == 3)

--- update & query single elements like a regular table
local s13 = s{1,2,3}
assert(s13[2] == 2) -- query element
assert(s13() == 1)
assert(s13() == 2)
assert(s13() == 3)
s13[1] = 4
assert(s13() == 4)
assert(s13() == 2)
assert(s13() == 3)

--- update the whole table
local s14 = s{1,2,3,4}
assert(s14() == 1)
assert(s14() == 2)
assert(s14() == 3)
s14:settable{5,6}
assert(s14() == 6) -- index folds into new range
assert(s14() == 5)
assert(s14() == 6)

local s8 = s{1, s{2}:every(2)}
assert(s8() == 1)
assert(s8() == 1)
assert(s8() == 2)
assert(s8() == 1)
assert(s8() == 1)
assert(s8() == 2)

local s9 = s{1, s{2}:count(2)}
assert(s9() == 1)
assert(s9() == 2)
assert(s9() == 2)
assert(s9() == 1)
assert(s9() == 2)
assert(s9() == 2)
assert(s9() == 1)

local s10 = s{ s{1,2}:all(), s{3,4}:all()}
assert(s10() == 1)
assert(s10() == 2)
assert(s10() == 3)
assert(s10() == 4)
assert(s10() == 1)
assert(s10() == 2)
assert(s10() == 3)
assert(s10() == 4)

local s11 = s{1, s{2}:every(2), s{3}:count(3)}
assert(s11() == 1)
assert(s11() == 3)
assert(s11() == 3)
assert(s11() == 3)
assert(s11() == 1)
assert(s11() == 2)
assert(s11() == 3)
assert(s11() == 3)
assert(s11() == 3)
assert(s11() == 1)

local s12 = s{1, s{2,3}:all():every(2)}
assert(s12() == 1)
assert(s12() == 1)
assert(s12() == 2)
assert(s12() == 3)
assert(s12() == 1)
assert(s12() == 1)
assert(s12() == 2)
assert(s12() == 3)
assert(s12() == 1)

-- --- naming nested sequins
-- -- once reaching a certain level of complexity, it's easier to name sequins before nesting
local melody = s{3,4,5}:all()
local umelo = s{-3,-4,-5}:all()
local s15 = s{ melody
             , umelo:every(3)
             }
assert(s15() == 3)
assert(s15() == 4)
assert(s15() == 5)
assert(s15() == 3)
assert(s15() == 4)
assert(s15() == 5)
assert(s15() == 3)
assert(s15() == 4)
assert(s15() == 5)
assert(s15() == -3)
assert(s15() == -4)
assert(s15() == -5)
assert(s15() == 3)
assert(s15() == 4)
assert(s15() == 5)


--- wrapped iteration should multiply calls like nested loops
local s16 = s{1, s{2}:count(2):count(2)}
assert(s16() == 1)
assert(s16() == 2)
assert(s16() == 2)
assert(s16() == 2)
assert(s16() == 2)
assert(s16() == 1)

-- FIXME broken- current result is (1,2,1,3,1,2,1,3)
--- reverse nested every/count
-- local s17 = s{1, s{2,3}:every(2):count(3)}
-- for i=1,8 do print(); print(s17()) end
-- assert(s17() == 1)
-- assert(s17() == 2)
-- assert(s17() == 3)
-- assert(s17() == 2)
-- assert(s17() == 1)
-- assert(s17() == 3)
-- assert(s17() == 2)
-- assert(s17() == 3)
-- assert(s17() == 1)

-- --- every+every composition divides timing
-- FIXME disabled bc i can't tell what the desired behaviour is
-- local s17 = s{1, s{2,3}:every(2):every(2)}
-- for i=1,8 do print(); print(s17()) end
-- assert(s17() == 1)
-- assert(s17() == 1)
-- assert(s17() == 1)
-- assert(s17() == 1)
-- assert(s17() == 2)
-- assert(s17() == 1)
-- assert(s17() == 1)
-- assert(s17() == 1)
-- assert(s17() == 1)
-- assert(s17() == 3)
-- assert(s17() == 1)

local s18 = s{ 1, s{2,3}:all():count(2)}
assert(s18() == 1)
assert(s18() == 2)
assert(s18() == 3)
assert(s18() == 2)
assert(s18() == 3)
assert(s18() == 1)
assert(s18() == 2)
assert(s18() == 3)
assert(s18() == 2)

local s19 = s{ 1, s{2,3}:times(3)}
assert(s19() == 1)
assert(s19() == 2)
assert(s19() == 1)
assert(s19() == 3)
assert(s19() == 1)
assert(s19() == 2)
assert(s19() == 1)
assert(s19() == 1)
assert(s19() == 1)

-- reassign next behaviour
local s5 = s{1,2,3,4}
assert(s5() == 1)
assert(s5() == 2)
s5:step(-1)
assert(s5() == 1)
assert(s5() == 4) -- wraps


-- generic cond function
STATE = true
local s1 = s{1, s{2}:cond(function() return STATE end)}
assert(s1() == 1)
assert(s1() == 2)
assert(s1() == 1)
assert(s1() == 2)
STATE = false
assert(s1() == 1)
assert(s1() == 1)
assert(s1() == 1)


-- test reset
local s18 = s{ 1, s{2,3}:all():count(2)}
assert(s18() == 1)
assert(s18() == 2)
assert(s18() == 3)
assert(s18() == 2)
s18:reset()
assert(s18() == 1)
assert(s18() == 2)
assert(s18() == 3)
assert(s18() == 2)
assert(s18() == 3)
assert(s18() == 1)
