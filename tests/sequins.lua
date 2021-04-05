--- sequins.lua tester

s = sequins

--- make a table of notes, with default next() behaviour
local s1 = s{0,4,7,11}
s1() --> 0
s1() --> 4
s1() --> 7
s1() --> 11
s1() --> 0 (wraps)

--- custom behaviour: steps backward by 1
local s2 = s({0,3,6,9}, s.step(-1))
s2() --> 0
s2() --> 9 (wraps)
s2() --> 6
s2() --> 3

--- default behaviour, demonstrate select for direct setting
local s3 = s{0,1,2,3,4,5}
s3() --> 0
s3() --> 1
s3:select(4) --> 4
s3() --> 5
s3() --> 0 (wraps)

--- explicit behaviours (any datatype!)
local s4 = s{'a','b','c','d'}
s4:select(0) --> 'a'
s4:select(0) --> 'a'
s4:select(2) --> 'c'
s4:step(0) --> 'c'
s4:step(2) --> 'a' (wraps)
s4:step(2) --> 'c'
s4:step(-1) --> 'b'

--- reassign next behaviour
local s5 = s{1,2,3,4}
s5() --> 1
s5() --> 2
s5.next = s.step(-1)
s5() --> 1
s5() --> 4 (wraps)

--- custom next fn
local s6rand = s({1,2,3}, function(self) self:select(math.random(1, self.length)) end)
local s6drnk = s({1,2,3}, function(self) self:step(math.random(-1, 1)) end)
-- UNKNOWN. determine calling convention after we've written .step and .next etc
-- s6.next = function(self)  

--- nesting sequins
local s7 = s{1, s{2, 3, 4}}
s7() --> 1
s7() --> 2
s7() --> 1. Note how it interleaves nested tables. ie. outer sequence has precedence
s7() --> 3
s7() --> 1
s7() --> 4

--- sequencing modifiers
local s8 = s{1, s.every(2, s{2}), s.count(3, s{3})}
s8() --> 1
    -- NB! we skip the s.every bc it's the 2nd call
s8() --> 3
s8() --> 3
s8() --> 3
s8() --> 1
s8() --> 2 (this time it occurs bc it's the 2nd count)
s8() --> 3

--- alternate style
local s8 = s{ 1
            , s.every(2, s{2})
            , s.count(3, s{3})
            }

--- second alternate style
local s8 = s{ 1
            , s{2}:s.every(2)
            , s{3}:s.count(3)
            }

--- chaining modifiers
local s9 = s{1, 2, s{3,4,5}:s.all():s.every(2)}
s9() --> 1
s9() --> 2
    -- 3rd element skips due to every
s9() --> 1
s9() --> 2
    -- 3rd element now activates & counts 3 times (ie all())
s9() --> 3
s9() --> 4
s9() --> 5
    -- repeats
s9() --> 1


--- naming nested sequins
-- once reaching a certain level of complexity, it's easier to name sequins before nesting
local melody = s{3,4,5}:s.all() -- chaining s.all() forces playback to run *all* elements before releasing control
local umelo = s{-3,-4,-5}:s.all()
local s10 = s{ melody -- all() == count(#self)
             , umelo:s.every(3) -- every 3rd time, insert the whole umelody
             }

--- __call() shortcut is just 'self:next()'
local s11 = s{1,2,3,4}
s11:next() --> 1 -- behaviour is identical
s11() --> 2


---- what if behaviours could take sequins as args
local s12 = s({1,2,3}, s.step(s{1,1,-1}))
-- this is really weird, but it makes it such that sequins become first class value/object dualities





