-- add timeline support

local s = sequins
local clk = clock

local TL = {}

-- helper fns
local real = function(q) return s.is_sequins(q) and q() or q end
local apply = function(fn, ...) fn(...) end
local bsleep = function(v) clk.sleep(clk.get_beat_sec(v)) end
local isfn = function(f) return (type(f) == 'function') end

-- abstract fns that handle value realization & fn application
local doact = function(fn)
    fn = real(fn)
    if isfn(fn) then fn()
    else -- table of fn & args
        local t = {} -- make a copy to avoid changing sequins
        for i=1,#fn do t[i] = real(fn[i]) end
        apply(table.unpack(t))
    end
end

local dosync = function(v) clk.sync(real(v)) end
    -- FIXME dowait will gradually un-sync (need extended clocks)
local dowait = function(v) bsleep(real(v)) end

local doalign = function(b, z)
    local ct = clk.get_beats() - z -- zero-ref'd current time
    b = real(b)
    if ct < b then bsleep(b-ct) end
end

local dopred = function(p)
    p = real(p) -- realize sequins value
    return isfn(p) and p() or p
end

-- core timeline fns
function TL.loop(t)
    local l = {}
    l.s = s(t) -- sequins for easy traversal & looping
    l.p = false -- predicate val/seqn/fn
    local lq = 1 -- launch quantize
    l.coro = clk.run(function()
        clk.sync(lq) -- launch quantization
        while true do
            local dur, act = l.s(), l.s()
            doact(act)
            dowait(dur)
            if dopred(l.p) then break end
        end
    end)
    -- TODO setmetatable for :stop and other methods
    return l
end

-- predicate chains
-- TODO these are *only* in metamethods of created tl.loop objects
function TL:unless(pred)
    self.p = pred
    return self -- method chain
end
function TL:once()
    self.p = true
    return self -- method chain
end
function TL:times(n)
    self.p = function()
        n = n - 1
        return (n == 0) -- true at minimum count
    end
    return self -- method chain
end

-- metamethod for any created tl object
function TL.stop(l)
    clk.cancel(l.coro)
    l = {} -- explicitly destroy the table
end

function TL.score(t)
    local l = {}
    l.coro = clk.run(function()
        local lq = 1 -- launch quantization. TODO configure externally
        local ct = lq - (clk.get_beats() % lq) -- find delay before first beat
        local zb = clk.get_beats() + ct -- calculate beat-zero
        for i=1,#t,2 do
            local ts, act = t[i], t[i+1]
            doalign(ts, zb)
            doact(act)
        end
    end)
    return l
end

--- metamethods

TL.__call = function(self, ...) return TL.loop(...) end

-- TL.__index = function(self, ix) end
-- TL.__newindex = function(self, ix, v) end

setmetatable(TL, TL)

return TL
