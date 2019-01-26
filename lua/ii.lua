--- Crow i2c library for lua frontend
--

local ii = {}

function ii.listmodules()
    ii_list_modules()
end











--function ii.setup()
--    -- sets stm32 i2c rx address?
--    -- TODO allow user to change i2c address so crow can pretend to be
--    -- another module by redefining ii.callback() with handlers for
--    -- the received data
--    --
--    -- TODO
--    -- big idea is that this fn could take the name of the destination
--    -- module ('Just Friends' or 'ansible') and *generate* the stub
--    -- fns for that module.
--    -- also returning the name of that module's functions in the repl.
--    -- the user could then use custom fns per destination
--    --
--    -- strike this as the sender often wants to send to multiple dest's
--    -- and thus it makes no sense to only have a small subset avail
--end
--
--function ii.send( destination )
--    -- consider whether the TT commands could be aliased to something
--    -- more in the lua/crow style like carp lets u rename C libs.
--    --
--    -- there could be the option of JT.BPM but also justtype.set_tempo
--end
--
---- should keep a reference of i2c addresses to accompanying fns
---- might be an issue to have *all* the i2c address combos have
---- stub functions. (bc i think lua program is loaded into ram,
---- and having 500 extra fns probably uses a good chunk)
---- instead, the diff modules are diff source files which can
---- be loaded with dofile() when the i2c address is set
--
--ii.cb_list = { ['crow'] = {1 = } }
--
--function ii.callback( addr, cmd, ... )
--    -- if addr is in ii.cb_list
--
--
--    -- CW.OUT       chan (value)-> (value)
--    -- CW.OUT.SLEW  chan (slew) -> (slew)
--    -- CW.IN        chan        -> input_val
--    -- CW.CALL      1arg
--    -- CW.CALL2     2args
--    -- CW.CALL3     3args
--    -- CW.CALL4     4args
--    --      triggers a callback in lua for the user to customize
--    --      specific meaning. if 1-less-than-expected in TT it is
--    --      considered a request and crow should return a value.
--end
--
--function ii
--
---- triggered by received ii msg 'CW.CALL'
--function ii.call1( arg )
--    if arg == nil then  -- this is a data request
--        ii.send()       -- return *something* to leader
--    else
--                        -- this is where you define your action
--    end
--end
--
--ii.callback_list = [[
--
--if the last arg is nil, this is a *request*
--   return something with ii.send()
--CW.CALL  -> function ii.call1( arg ) end
--CW.CALL2 -> function ii.call2( arg, arg2 ) end
--CW.CALL3 -> function ii.call3( arg, arg2, arg3 ) end
--CW.CALL4 -> function ii.call4( arg, arg2, arg3, arg4 ) end
--]]
--
---- prints a list of connected devices
--function ii.get_devices()
--    -- requires update to the ii tech across the board
--    -- 1. send 'ping' to universal channel
--    -- 2. record responses in a table
--    -- 3. print a list of those responses (with id numbers?)
--end
--
---- prints a list of available (lua) commands for a given device
--function ii.get_commands( devicename )
--    -- this is the list of functions that are implemented for crow
--    --print(debug.getinfo(ii.call, 'n').)
--end

print 'ii loaded'

return ii
