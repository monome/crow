get_offset = 0x80

ii_address = ''

function prepare_multi_address(f)
    local h = ''
    if type(f.i2c_address) == 'table' then
        h = h .. 'local _addrs={'
        for _,v in ipairs(f.i2c_address) do
            h = h .. v .. ','
        end
        h = h .. '}\n'
            .. 'local _addr=' .. f.i2c_address[1] .. '\n'
            .. 'local _ix=1\n\n'
        ii_address = '_addr'
    else
        ii_address = f.i2c_address
    end
    return h
end

function lua_cmds(f)
    if not f.commands then return '' end
    local c = ''
    for _,v in ipairs( f.commands ) do
        c = c .. 'function ' .. f.lua_name .. '.' .. v.name .. '(...)ii.set('
          .. ii_address .. ',' .. v.cmd .. ',...)end\n'
    end
    return c .. '\n'
end

function lua_getters(f)
    local g = f.lua_name .. '.g={\n'
    if f.commands then
        for _,v in ipairs( f.commands ) do
            if v.get == true then
                g = g .. '\t[\'' .. v.name .. '\']=' .. (v.cmd + get_offset) .. ',\n'
            end
        end
    end
    if f.getters ~= nil then
        for _,v in ipairs( f.getters ) do
            if type(v.name) == 'number' then
                g = g .. '\t[' .. v.name .. ']=' .. v.cmd .. ',\n'
            else
                g = g .. '\t[\'' .. v.name .. '\']=' .. v.cmd .. ',\n'
            end
        end
    end
    g = g .. '}\n'

    g = g .. 'function ' .. f.lua_name .. '.get(name,...)ii.get('
      .. ii_address .. ',' .. f.lua_name .. '.g[name],...)end\n\n'
    return g
end

function lua_events(f)
    local e = f.lua_name .. '.e={\n'
    if f.commands then
        for _,v in ipairs( f.commands ) do
            if v.get == true then
                e = e .. '\t[' .. (v.cmd + get_offset) .. ']=\'' .. v.name .. '\',\n'
            end
        end
    end
    if f.getters ~= nil then
        for _,v in ipairs( f.getters ) do
            if type(v.name) == 'number' then
                e = e .. '\t[' .. v.cmd .. ']=' .. v.name .. ',\n'
            else
                e = e .. '\t[' .. v.cmd .. ']=\'' .. v.name .. '\',\n'
            end
        end
    end
    e = e .. '}\n'
          .. 'function ' .. f.lua_name
    e = e .. '.event(e,data)ii.e(\'' .. f.lua_name .. '\',e,data)end\n\n'
    return e
end

function lua_meta(f)
    if type(f.i2c_address) == 'number' then return '' end
    return f.lua_name .. '.__index=function(self,ix)\n'
            .. '\tif type(ix)==\'number\' then\n'
            .. '\t\t_ix=ix\n'
            .. '\t\t_addr=_addrs[ix]\n'
            .. '\t\tif not _addr then\n'
            .. '\t\t\t_ix=1\n'
            .. '\t\t\t_addr=_addrs[ix]\n'
            .. '\t\tend\n'
            .. '\t\treturn self\n'
            .. '\tend\n'
            .. 'end\n'
            .. 'setmetatable(' .. f.lua_name .. ',' .. f.lua_name .. ')\n\n'
end

function make_lua(f)
    local l = 'local ' .. f.lua_name .. '={}\n\n'
            .. prepare_multi_address(f)
            .. lua_cmds(f)
            .. lua_getters(f)
            .. lua_events(f)
            .. lua_meta(f)
            .. 'return ' .. f.lua_name .. '\n'
    return l
end

local in_file = arg[1]
local out_file = arg[2]
do
    o = io.open( out_file, 'w' )
    o:write( make_lua( dofile( in_file)))
    o:close()
end

-- example usage:
-- lua util/ii_lua_module.lua lua/ii/jf.lua build/ii_jf.lua
