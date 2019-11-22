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

function ii_LeadRx_handler( addr, cmd, data )
    local name = ii.is.lu[addr]
    ii[name].event(ii[name].e[cmd], data)
end

-- NOTE: weird double-escaped quotes down here for the c compiler
function ii.e( name, event, data ) crow.tell('ii.'..name,'\\''..tostring(event)..'\\'',data) end

ii.self =
    { cmds = { [1]='output'
             , [2]='slew'
             , [4]='call1'
             , [5]='call2'
             , [6]='call3'
             , [7]='call4'
             -- input & output queries in C only
             , [5+128]='query0'
             , [6+128]='query1'
             , [7+128]='query2'
             , [8+128]='query3'
             }
    , output = function(chan,val) print('output '..chan..' to '..val)end
    , slew = function(chan,slew) print('slew '..chan..' at '..slew)end
    , call1 = function(arg) print('call1('..arg..')')end
    , call2 = function(a,a2) print('call2('..a..','..a2..')')end
    , call3 = function(a,a2,a3) print('call3('..a..','..a2..','..a3..')')end
    , call4 = function(a,a2,a3,a4) print('call4('..a..','..a2..','..a3..','..a4..')')end

    , query0 = function() print('query0()'); return 5 end
    , query1 = function(a) print('query1('..a..')'); return 6 end
    , query2 = function(a,a2) print('query2('..a..','..a2..')'); return 7 end
    , query3 = function(a,a2,a3) print('query3('..a..','..a2..','..a3..')')
        return 8
    end
}

function ii_followRx_handler( cmd, ... )
    local name = ii.self.cmds[cmd]
    ii.self[name](...)
end

function ii_followRxTx_handler( cmd, ... )
    local name = ii.self.cmds[cmd]
    return ii.self[name](...)
end

return ii
