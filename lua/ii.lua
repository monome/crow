--- Crow i2c library for lua frontend
--

local ii = {}

ii.is = dofile('build/iihelp.lua')

--- METAMETHODS
ii.__index = function( self, ix )
    local e = rawget(ii.is, ix) -- avoids openlib() in case of .help
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
    if state == true then state = 1 else state = 0 end
    ii_pullup(state)
end

-- aliases to C functions
ii.set_address = ii_set_add
ii.get_address = ii_get_add

-- TODO is it possible to just define ii.lead from c directly?
function ii.set( address, cmd, ... )
    ii_lead( address, cmd, ... )
end

function ii.get( address, cmd, ... )
    if not cmd then print'param not found'
    else ii_lead( address, cmd, ... ) end
end

function ii_LeadRx_handler( addr, cmd, _arg, data )
    local name, ix = ii.is.lookup(addr)
    local rx_event = { name   = ii[name].e[cmd]
                     , device = ix or 1
                     , arg    = _arg
                     }
    ii[name].event(rx_event, data)
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
    ii.self.volts  = function(chan,val) print('volts '..chan..' to '..val)end
    ii.self.slew   = function(chan,slew) print('slew '..chan..' at '..slew)end
    ii.self.call1  = function(arg) print('call1('..arg..')')end
    ii.self.call2  = function(a,a2) print('call2('..a..','..a2..')')end
    ii.self.call3  = function(a,a2,a3) print('call3('..a..','..a2..','..a3..')')end
    ii.self.call4  = function(a,a2,a3,a4) print('call4('..a..','..a2..','..a3..','..a4..')')end
    ii.self.query0 = function() print('query0()'); return 5 end
    ii.self.query1 = function(a) print('query1('..a..')'); return 6 end
    ii.self.query2 = function(a,a2) print('query2('..a..','..a2..')'); return 7 end
    ii.self.query3 = function(a,a2,a3) print('query3('..a..','..a2..','..a3..')')
        return 8
    end
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

return ii
