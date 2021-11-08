--- Crow i2c library for lua frontend
--

local ii = {}

ii.is = dofile('build/iihelp.lua')

function ii.help()
    ii_list_modules()
end

function ii.m_help( address )
    ii_list_commands( address )
end

function ii.pullup( state )
    if state == true then state = 1 else state = 0 end
    ii_pullup(state)
end

-- aliases to C functions
ii.set_address = ii_set_add
ii.get_address = ii_get_add
ii.set = ii_lead

function ii.raw( address, bytes, rx_len )
    ii_lead_bytes( address, bytes, rx_len or 0 )
end

function ii.get( address, cmd, ... )
    if not cmd then print'param not found'
    else ii_lead( address, cmd, ... ) end
end

function ii_LeadRx_handler( addr, cmd, _arg, data )
    if ii.event_raw(addr, cmd, data, arg) then return end
    local name, ix = ii.is.lookup(addr)
    if name ~= nil then
        local rx_event = { name   = ii[name].e[cmd]
                         , device = ix or 1
                         , arg    = _arg
                         }
        ii[name].event(rx_event, data)
    end
end

function ii.e( name, event, ... )
    local e_string = '{name=[['..event.name..']]'
                  .. ',device='..event.device
                  .. ',arg='..event.arg
                  .. '}'
    crow.tell('ii.'..name,e_string,...)
end

ii.self =
    { cmds = { [1]='volts'
             , [2]='slew'
             , [4]='call1'
             , [5]='call2'
             , [6]='call3'
             , [7]='call4'
             , [8]='reset'
             , [9]='pulse'
             , [10]='ar'
             , [11]='lfo'
             -- input & output queries in C only
             , [5+128]='query0'
             , [6+128]='query1'
             , [7+128]='query2'
             , [8+128]='query3'
             }
    }
function ii.reset_events()
    ii.self.call1  = function(a) ii.self.call{a} end
    ii.self.call2  = function(a,a2) ii.self.call{a,a2} end
    ii.self.call3  = function(a,a2,a3) ii.self.call{a,a2,a3} end
    ii.self.call4  = function(a,a2,a3,a4) ii.self.call{a,a2,a3,a4} end
    ii.self.query0 = function() return ii.self.query{} end
    ii.self.query1 = function(a) return ii.self.query{a} end
    ii.self.query2 = function(a,a2) return ii.self.query{a,a2} end
    ii.self.query3 = function(a,a2,a3) return ii.self.query{a,a2,a3} end

    -- all the individual call/queries forward to these 2 user fns
    ii.self.call = function(args) print('call'..#args) end
    ii.self.query = function(args) print('query'..#args); return 0 end

    ii.event_raw = function(addr, cmd, data, arg) end
end
ii.reset_events()

function ii_followRx_handler( cmd, ... )
    local name = ii.self.cmds[cmd]
    ii.self[name](...)
end

function ii_followRxTx_handler( cmd, ... )
    local name = ii.self.cmds[cmd]
    return ii.self[name](...)
end

--- METAMETHODS
ii.__index = function( self, ix )
    if ix == 'address' then return ii.get_address() end
    local e = rawget(ii.is, ix) -- avoids openlib() in case of .help
    if e ~= nil then return e
    else print'not found. try ii.help()' end
end
ii.__newindex = function( self, ix, v )
    if ix == 'address' then ii.set_address(v)
    else rawset(self, ix, v) end
end
setmetatable(ii, ii)

return ii
