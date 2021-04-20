--- clock coroutines
-- @module clock

local clock = { threads = {}
              , transport = {}
              , internal = {}
              , crow = {}
              , id = 0
              }

--- create a coroutine to run but do not immediately run it;
-- @tparam function f
-- @treturn integer : coroutine ID that can be used to resume/stop it later
clock.create = function(f)
  local coro = coroutine.create(f)
  clock.id = clock.id + 1 -- create a new id
  clock.threads[clock.id] = coro
  return clock.id
end

--- create a coroutine from the given function and immediately run it;
-- the function parameter is a task that will suspend when clock.sleep and clock.sync are called inside it and will wake up again after specified time.
-- @tparam function f
-- @treturn integer : coroutine ID that can be used to stop it later
clock.run = function(f, ...)
  local coro_id = clock.create(f)
  clock.resume(coro_id, ...)
  return coro_id
end

--- stop execution of a coroutine started using clock.run.
-- @tparam integer coro_id : coroutine ID
clock.cancel = function(coro_id)
  clock_cancel(coro_id)
  clock.threads[coro_id] = nil
end

-- enum for calls to C
local SLEEP = 0
local SYNC = 1
local SUSPEND = 2

--- yield and schedule waking up the coroutine in s seconds;
-- must be called from within a coroutine started with clock.run.
-- @tparam float s : seconds
clock.sleep = function(...)
  return coroutine.yield(SLEEP, ...)
end

--- yield and schedule waking up the coroutine at beats beat;
-- the coroutine will suspend for the time required to reach the given fraction of a beat;
-- must be called from within a coroutine started with clock.run.
-- @tparam float beats : next fraction of a beat at which the coroutine will be resumed. may be larger than 1.
clock.sync = function(...)
  return coroutine.yield(SYNC, ...)
end

--- yield and do not schedule wake up, clock must be explicitly resumed
-- must be called from within a coroutine started with clock.run.
clock.suspend = function()
  return coroutine.yield(SUSPEND)
end

clock.resume = function(coro_id, ...)
  local coro = clock.threads[coro_id]

  if coro == nil then
    print('cant resume cancelled clock')
    return
  end

  local result, mode, time = coroutine.resume(clock.threads[coro_id], ...)

  if coroutine.status(coro) == 'dead' then
    if result then
      clock.cancel(coro_id)
    else
      print('error: ' .. mode)
    end
  else
    -- not dead
    if result and mode ~= nil then
      if mode == SLEEP then
        clock_schedule_sleep(coro_id, time)
      elseif mode == SYNC then
        clock_schedule_sync(coro_id, time)
      -- elseif mode == SUSPEND then
        -- nothing needed for SUSPEND
      end
    end
  end
end

clock.cleanup = function()
  for id, coro in pairs(clock.threads) do
    if coro then
      clock.cancel(id)
    end
  end
  clock.transport.start = nil
  clock.transport.stop = nil
end

--- select the sync source
-- @tparam string source : 'internal', 'midi', 'link' or 'crow'
clock.set_source = function(source)
  if source == 'internal' then
    clock_set_source(1)
  elseif source == 'crow' then
    clock_set_source(4)
  else
    print('unknown clock source: '..source)
  end
end

clock.get_beats = clock_get_time_beats
clock.get_tempo = clock_get_tempo
clock.get_beat_sec = function(x) return (x or 1) * 60.0 / clock.get_tempo() end

clock.internal.set_tempo = function(bpm) return clock_internal_set_tempo(bpm) end
clock.internal.start = function(beat) return clock_internal_start(beat or 0) end
clock.internal.stop = clock_internal_stop


-- event handlers (called from C)
clock_resume_handler = clock.resume
function clock_start_handler() safe_call(clock.transport.start) end
function clock_stop_handler() safe_call(clock.transport.stop) end

return clock
