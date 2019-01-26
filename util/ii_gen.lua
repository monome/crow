get_offset = 0x80

-- generate a printable c-string describing the module's commands
function generate_prototypes( d )
    local prototypes = ''
    local proto_prefix = 'ii.' .. d.lua_name .. '.'
    for _,v in ipairs( d.commands ) do
        local s = proto_prefix .. v.name .. '( '
        if type(v.args[1]) == 'table' then -- more than 1 arg
            local arg_count = #(v.args)
            for i=1,arg_count do
                s = s .. v.args[i][1]
                if i ~= arg_count then s = s .. ', ' end
            end
        else -- only 1 arg
            s = s .. v.args[1]
        end
        s = s .. ' )\n\r'
        prototypes = prototypes .. s
    end
    return prototypes
end

function has_getters( d )
    if d.getters ~= nil then return true
    else
        for _,v in ipairs( d.commands ) do
            if v.get == true then return true end
        end
    end
    return false
end

function generate_getters( d )
    local getters = ''
    local get_prefix = 'ii.' .. d.lua_name .. '.get( \''

    -- commands that have standard getters
    for _,v in ipairs( d.commands ) do
        if v.get == true then
            local s = get_prefix .. v.name .. '\''
            if v.args ~= nil then
                if type(v.args[1]) == 'table' then -- >1 arg
                    for i=1,#(v.args)-1 do -- ignore last
                        s = s .. ', ' .. v.args[i][1]
                    end
                end -- if only 1 arg it is ignored!
            end
            s = s .. ' )\n\r'
            getters = getters .. s
        end
    end

    -- get-only commands, or custom getters
    for _,v in ipairs( d.getters ) do
        local s = get_prefix .. v.name .. '\''
        if v.args ~= nil then
            if type(v.args[1]) == 'table' then -- multiple args
                for i=1,#(v.args)-1 do -- ignore last
                    s = s .. ', ' .. v.args[i][1]
                end
            else s = s .. ', ' .. v.args[1] end
        end
        s = s .. ' )\n\r'
        getters = getters .. s
    end
    return getters
end

function generate_events( d )
    local has_get_cmd = function(d)
        for _,v in ipairs( d.commands ) do
            if v.get == true then return true end
        end
        return false
    end

    local events = 'ii.' .. d.lua_name .. '.event = function( e, data )\n\r'

    local overloaded = has_get_cmd(d)
    if overloaded then
        local heading = true
        for _,v in ipairs( d.commands ) do
            if v.get == true then
                if heading then
                    events = events .. '  if e == \'' .. v.name .. '\' then\n\r'
                           .. '    -- handle ' .. v.name .. ' param here\n\r'
                    heading = false
                else
                    events = events .. '  elseif e == \'' .. v.name .. '\' then\n\r'
                end
            end
        end
    else
        events = events .. '  if e == \'' .. d.getters[1].name .. '\' then\n\r'
               .. '    -- handle ' .. d.getters[1].name .. ' param here\n\r'
    end

    for i=(overloaded and 1 or 2),#(d.getters) do
        events = events .. '  elseif e == \'' .. d.getters[i].name .. '\' then\n\r'
    end
    events = events .. '  end\n\r'
             .. 'end'
    return events
end

function make_help(f)
    local h = '-- commands\n\r'
    h = h .. generate_prototypes(f) .. '\n\r'
    if has_getters(f) then
        h = h .. '-- request params\n\r'
        h = h .. generate_getters(f) .. '\n\r'
        h = h .. '-- then receive\n\r'
        h = h .. generate_events(f) .. '\n\r'
    end
    return h
end

function lua_cmds(f)
    local c = ''
    for _,v in ipairs( f.commands ) do
        c = c .. 'function ' .. f.lua_name .. '.' .. v.name .. '(...)II.set('
          .. f.i2c_address .. ',' .. v.cmd .. ',...)end\n\r'
    end
    return c
end

function lua_getters(f)
    local g = f.lua_name .. '.g={\n'
    for _,v in ipairs( f.commands ) do
        if v.get == true then
            g = g .. '\t[\'' .. v.name .. '\']=' .. (v.cmd + get_offset) .. ',\n\r'
        end
    end
    for _,v in ipairs( f.getters ) do
        g = g .. '\t[\'' .. v.name .. '\']=' .. v.cmd .. ',\n\r'
    end
    g = g .. '}\n\r'

    g = g .. 'function ' .. f.lua_name .. '.get(name,...)II.get('
      .. f.i2c_address .. ',' .. f.lua_name .. '.g[name],...)end\n\r'
    return g
end

function lua_events(f)
    local e = f.lua_name .. '.e={\n'
    for _,v in ipairs( f.commands ) do
        if v.get == true then
            e = e .. '\t[' .. (v.cmd + get_offset) .. ']=\'' .. v.name .. '\',\n\r'
        end
    end
    for _,v in ipairs( f.getters ) do
        e = e .. '\t[' .. v.cmd .. ']=\'' .. v.name .. '\',\n\r'
    end
    e = e .. '}\n\r'
    return e
end

function make_lua(f)
    local l = 'local ' .. f.lua_name .. '={}\n\n\r'
    l = l .. lua_cmds(f)
    l = l .. lua_getters(f)
    l = l .. lua_events(f)
    l = l .. 'return ' .. f.lua_name .. '\n\r'
    return l
end


function make_c(f)
    local c = ''
    -- c needs to have a set of lookup tables with the arg types
    -- it will need to do conversion from floats into integer representation
    -- furthermore getters need to know how many bytes to expect
    -- and then how to convert those bytes back to data
    return c
end


-- TODO just iterate through the lua/ii/ folder making a new pirate for each
pirate = dofile('lua/ii/jf.lua')

print('> ii.' .. pirate.lua_name .. '.help()\n\r')
print(make_help(pirate))

print('--- module\'s lua')
print(make_lua(pirate))

print('--- module\'s c')
print(make_c(pirate))
