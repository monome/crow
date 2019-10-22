get_offset = 0x80

function lua_cmds(f)
    local c = ''
    for _,v in ipairs( f.commands ) do
        c = c .. 'function ' .. f.lua_name .. '.' .. v.name .. '(...)ii.set('
          .. f.i2c_address .. ',' .. v.cmd .. ',...)end\n'
    end
    return c
end

function lua_getters(f)
    local g = f.lua_name .. '.g={\n'
    for _,v in ipairs( f.commands ) do
        if v.get == true then
            g = g .. '\t[\'' .. v.name .. '\']=' .. (v.cmd + get_offset) .. ',\n'
        end
    end
    if f.getters ~= nil then
        for _,v in ipairs( f.getters ) do
            g = g .. '\t[\'' .. v.name .. '\']=' .. v.cmd .. ',\n'
        end
    end
    g = g .. '}\n'

    g = g .. 'function ' .. f.lua_name .. '.get(name,...)ii.get('
      .. f.i2c_address .. ',' .. f.lua_name .. '.g[name],...)end\n'
    return g
end

function lua_events(f)
    local e = f.lua_name .. '.e={\n'
    for _,v in ipairs( f.commands ) do
        if v.get == true then
            e = e .. '\t[' .. (v.cmd + get_offset) .. ']=\'' .. v.name .. '\',\n'
        end
    end
    if f.getters ~= nil then
        for _,v in ipairs( f.getters ) do
            e = e .. '\t[' .. v.cmd .. ']=\'' .. v.name .. '\',\n'
        end
    end
    e = e .. '}\n'
    return e
end

function make_lua(f)
    local l = 'local ' .. f.lua_name .. '={}\n\n'
            .. lua_cmds(f)
            .. lua_getters(f)
            .. lua_events(f)
            .. 'function ' .. f.lua_name
                .. '.event(e,data)ii.e(\'' .. f.lua_name .. '\',e,data)end\n'
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
