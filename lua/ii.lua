--- Crow i2c library for lua frontend
--

local ii = {}

ii.is = dofile('build/iihelp.lua')

--- METAMETHODS
ii.__index = function( self, ix )
    local e = rawget(ii.is, ix)
    if e ~= nil then return e
    else print'not found. try ii.help()' end
end
setmetatable(ii, ii)

function ii.help()
    ii_list_modules()
end

function ii.m_help( address )
    ii_list_commands( address )
end

function ii.pullup( state )
    ii_pullup(state)
end

-- TODO is it possible to just define ii.set from c directly?
function ii.set( address, cmd, ... )
    ii_set( address, cmd, ... )
end

function ii.get( address, cmd, ... )
    ii_get( address, cmd, ... )
end

function ii_handler( addr, cmd, data )
    local name = ii.is.lu[addr]
    ii[name].event(ii[name].e[cmd], data)
end

ii._c =
    { cmds = { [1]='out'
             , [2]='slew'
             , [3]='input'
             , [4]='call'
             , [5]='call2'
             , [6]='call3'
             , [7]='call4'
             }
    , out = function(chan,val) print('out '..chan..' to '..val)end
    , slew = function(chan,slew) print('slew '..chan..' at '..slew)end
    , input = function(chan) print('input '..chan..' NEED RESPONSE')end
    , call = function(arg) print('call '..arg)end
    , call2 = function(a,a2) print('call2 '..a..' '..a2)end
    , call3 = function(a,a2,a3) print('call3 '..a..' '..a2..' '..a3)end
    , call4 = function(a,a2,a3,a4) print('call4 '..a..' '..a2..' '..a3..' '..a4)end
}

function ii_self_handler( cmd, ... )
    local name = ii._c.cmds[cmd]
    --ii._c[name](...)
end

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
--- triggered by received ii msg 'CW.CALL'
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

print 'ii loaded'

return ii
