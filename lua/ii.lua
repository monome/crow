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
    if state == true then state = 1 else state = 0 end
    ii_pullup(state)
end

-- TODO is it possible to just define ii.set from c directly?
function ii.set( address, cmd, ... )
    ii_set( address, cmd, ... )
end

function ii.get( address, cmd, ... )
    ii_get( address, cmd, ... )
end

function ii_LeadRx_handler( addr, cmd, data )
    local name = ii.is.lu[addr]
    ii[name].event(ii[name].e[cmd], data)
end

function ii.e( name, event, data ) _c.tell('ii.'..name,'\\''..tostring(event)..'\\'',data) end

ii._c =
    { cmds = { [1]='output'
             , [2]='slew'
             , [4]='call1'
             , [5]='call2'
             , [6]='call3'
             , [7]='call4'
             , [3+128]='input'
             , [4+128]='query0'
             , [5+128]='query1'
             , [6+128]='query2'
             , [7+128]='query3'
             , [8+128]='inputF'
             }
    , output = function(chan,val) print('output '..chan..' to '..val)end
    , slew = function(chan,slew) print('slew '..chan..' at '..slew)end
    , call1 = function(arg) print('call1('..arg..')')end
    , call2 = function(a,a2) print('call2('..a..','..a2..')')end
    , call3 = function(a,a2,a3) print('call3('..a..','..a2..','..a3..')')end
    , call4 = function(a,a2,a3,a4) print('call4('..a..','..a2..','..a3..','..a4..')')end

    , input = function(chan) print('input('..chan..')') return 3 end
    , query0 = function() print('query0()'); return 4 end
    , query1 = function(a) print('query1('..a..')'); return 5 end
    , query2 = function(a,a2) print('query2('..a..','..a2..')'); return 6 end
    , query3 = function(a,a2,a3) print('query3('..a..','..a2..','..a3..')')
        return 7
    end
    , inputF = function(chan) print('input('..chan..')') return 8 end
}

function ii_followRx_handler( cmd, ... )
    local name = ii._c.cmds[cmd]
    ii._c[name](...)
end

function ii_followRxTx_handler( cmd, ... )
    local name = ii._c.cmds[cmd]
    return ii._c[name](...)
end

print 'ii loaded'

return ii
