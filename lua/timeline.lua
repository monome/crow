--- timeline sequencer
-- hotrod some clock & sequins structures for rapid playability

--- globals are available on crow, otherwise require for norns
local s = sequins or require 'lib/sequins'
local clk = clock or require 'clock'

local TL = {}


-- create a timeline object from a table
function TL.new(t) return setmetatable(t, TL) end


-- helper fns
local real = function(q)
    if s.is_sequins(q) then return q() else return q end
end
local apply = function(fn, ...) fn(...) end
local bsleep = function(v) clk.sleep(clk.get_beat_sec()*v) end
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

-- UNUSED
-- local dosync = function(v) clk.sync(real(v)) end
    -- FIXME dowait will gradually un-sync (need extended clocks)

local dowait = function(d, z) -- duration, zero
    local z = z+real(d)
    clk.sync(z)
    return z
end

local doalign = function(b, z)
    local now = clk.get_beats()
    local ct = now - z -- zero-ref'd current time
    b = real(b)
    if ct < b then bsleep(b-ct) end
end

local doaligns = function(b, z)
    b = real(b) -- timestamp to wait until
    if b > z then
        bsleep(b-z) -- wait until the perfect moment
        return b -- return current time
    else return z end -- otherwise return time now
end

local dopred = function(p)
    p = real(p) -- realize sequins value
    if isfn(p) then return p() else return p end
end


--- pre-methods (chain into :loop etc)
-- works in all tl modes

-- launch quantization to lock to a clock.sync
function TL.launch(q) return TL.new{lq = q} end

-- stops auto-play
function TL.queue(q) return TL.new{qd = 1} end


--- loop
-- standalone
function TL.loop(t) return TL.new{}:loop(t) end

-- method version
function TL:_loop(t)
    self.mode = 'loop'
    self.fn = function()
        self.i = 0 -- iteration 0 before quant finished
        clk.sync(self.lq or 1) -- launch quantization
        self.z = math.floor(clk.get_beats()) -- reference beat to stop drift
        repeat
            self.i = self.i + 1
            for i=1,#t,2 do
                doact(t[i+1])
                self.z = dowait(t[i], self.z)
            end
        until(dopred(self.p or false))
    end
    if not self.qd then TL.play(self) end
    return self
end

-- loop predicate methods
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


--- score
-- standalone
function TL.score(t) return TL.new{}:score(t) end

-- method version
function TL:_score(t)
    self.mode = 'score'
    self.fn = function()
        local now = clk.get_beats()
        local lq = self.lq or 1
        self.z = now + (lq - (now % lq)) -- calculate beat-zero
        for i=1,#t,2 do
            doalign(t[i], self.z)
            doact(t[i+1])
        end
    end
    if not self.qd then TL.play(self) end
    return self
end


--- timed
-- standalone
function TL.timed(t) return TL.new{}:timed(t) end

-- method version
function TL:_timed(t)
    self.mode = 'timed'
    self.fn = function()
        clk.sync(self.lq or 1) -- launch quantization
        self.z = 0 -- tracks elapsed time as 0 is arbitrary
        for i=1,#t,2 do
            self.z = doaligns(t[i], self.z) -- track current loop time
            doact(t[i+1])
        end
    end
    if not self.qd then TL.play(self) end
    return self
end


--- post methods for operating on any tl object

-- stop a playing timeline
function TL:stop()
    clk.cancel(self.coro)
    self = {} -- explicitly destroy the table
end

-- play a queued timeline
function TL:play() self.coro = clk.run(self.fn) end

-- return count of loop repetitions inclusive
function TL:iter() return self.i end

-- alias clock cleanup to stop all running timelines
TL.cleanup = clk.cleanup

--- metamethods
TL.mms = { stop   = TL.stop
         , unless = TL.unless
         , times  = TL.times
         , once   = TL.once
         , loop   = TL._loop
         , score  = TL._score
         , timed  = TL._timed
         , play   = TL.play
         , iter   = TL.iter
         }
TL.__index = TL.mms

setmetatable(TL, TL)

return TL
