_c = { tell = function(...) print('ctell',...) end }
quotes = function(s) return string.format('%q',s) end

public = dofile("lua/public.lua")

--- numbers
-- plain integer
public.add('a', 1.1)
assert(public.a == 1.1)
public.a = 2.2
assert(public.a == 2.2)
public.update('a', 3.3)
assert(public.a == 3.3)

-- integer with limits
public.add('b', 1.1, {0.4,3.0})
public.b = 0.5
assert(public.b == 0.5)
public.b = -1
assert(public.b == 0.4)
public.b = 10
assert(public.b == 3.0)

-- integer with action
d = 0
public.add('c', 2, function(v) d = v end)
public.c = 3
assert(public.c == 3)
assert(d == 3)

-- integer with limits and action
f = 0
public.add('e', 10, {4,16}, function(v) f = v end)
public.e = 3
assert(f == 4)
public.e = 20
assert(f == 16)

--- string enums
-- enum
public.add('g','a',{'a','b','c'})
assert(public.g == 'a')
public.g = 'b'
assert(public.g == 'b')
public.g = 'z' -- out of range ignored
assert(public.g == 'b')

-- enum with action
i = ''
public.add('h','x',{'x','y','z'}, function(v) i = v end)
public.h = 'y'
assert(i == 'y')
public.h = 'f'
assert(i == 'y')

--- table of nums
-- plain table
tc = table.concat
public.add('j', {1,2,3,4})
assert( tc(public.j) == tc({1,2,3,4}))
public.j = {1,2,3}
assert( tc(public.j) == tc({1,2,3}))
public.update('j',{2,3})
assert( tc(public.j) == tc({2,3}))

-- table with action
l = {}
public.add('k', {1,2,3}, function(v) l = v end)
public.k = {3,4,5}
assert( tc(l) == tc{3,4,5})

-- table with limits
public.add('m', {6,5,4}, {1,8})
public.m = {6,5,4,20}
assert( tc(public.m) == tc{6,5,4,8})

-- table with limits and action
o = {}
public.add('n', {1,2,3}, {1,5}, function(v) o = v end)
public.n = {0,4,13,13}
assert( tc(o) == tc{1,4,5,5} )

--- indexed table
-- itable -> setting indices
public.add('p', {1,2,3,4})
assert(public.p.select(1) == 1)
assert(public.p.select(2) == 2)
assert(public.p.select() == 2)
assert(public.p.step() == 3)
assert(public.p.step(1) == 4)
assert(public.p.step(1) == 1)
assert(public.p.step(0) == 1) -- reselect current
assert(public.p.step(2) == 3)
assert(public.p.step(-1) == 2)
assert(public.p.step(-2) == 4)
assert(public.p.select() == 4) -- reselects most recent option
assert(public.p.step() == 2) -- maintains most recent step (-2)
assert(public.p.select(5) == 1) -- out of bounds should wrap
assert(public.p.select(6) == 2)

-- get itable indices (do we need this?)

-- remote update table members
public.update('p',3,'index')
assert(public.p.index == 3)
public.update('p',5,'index')
assert(public.p.index == 1)
public.update('p',5,3) -- update element 3 to value 5
assert(tc(public.p) == tc{1,2,5,4})

--- indexed table with limits & action
r={}
public.add('q',{1,2,3},{1,3},function(v)r=v end)
public.update('q', 3, 'index')
assert(public.q.index == 3)
public.update('q', 4, 2)
assert(tc(r) == tc{1,3,3})





