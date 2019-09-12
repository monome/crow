get_offset = 0x80

-- allows descriptor to use shorthand without stringifying
void = 'II_void'
s8 = 'II_s8'
u8 = 'II_u8'
s16 = 'II_s16'
s16V = 'II_s16V'
u16 = 'II_u16'
float = 'II_float'

function c_cmds(f)
    local c = ''

    local function make_a_cmd( lua_name, ix, cmd, args, retval )
        local s = 'const II_Cmd_t ' .. lua_name .. ix .. ' = {' .. cmd .. ','
        if args == nil then
            s = s .. '0,' .. retval .. ',{}'
        elseif type(args[1]) == 'table' then -- >1 arg
            s = s .. #args .. ',' .. retval .. ',{'
            for i=1,#args-1 do
                s = s .. args[i][2] .. ','
            end
            s = s .. args[#args][2] .. '}'
        elseif type(args[1]) == 'string' then -- 1 arg
            s = s .. '1,' .. retval .. ',{' .. args[2] .. '}'
        end
        return s .. '};\n'
    end

    local i = 0
    -- setters
    for _,v in ipairs( f.commands ) do
        c = c .. make_a_cmd( f.lua_name, i, v.cmd, v.args, void )
        i = i + 1
    end

    local function get_last_argtype( args )
        if type(args[1]) == 'table' then -- >1 arg
            return args[#args][2]
        elseif type(args[1]) == 'string' then -- 1 arg
            return args[2]
        end
        return void
    end


    local function drop_last_arg( args )
        if type(args[1]) == 'table' then -- >1 arg
            local r = {}
            for i=1,#args-1 do
                table.insert( r, args[i] )
            end
            return r
        else -- 1 or none
            return
        end
    end

    -- auto getters
    for _,v in ipairs( f.commands ) do
        if v.get == true then
            c = c .. make_a_cmd( f.lua_name
                               , i
                               , v.cmd + get_offset
                               , drop_last_arg( v.args )
                               , get_last_argtype( v.args )
                               )
            i = i + 1
        end
    end
    -- explicit getters
    if f.getters ~= nil then
        for _,v in ipairs( f.getters ) do
            c = c .. make_a_cmd( f.lua_name
                               , i
                               , v.cmd
                               , v.args
                               , v.retval[2]
                               )
            i = i + 1
        end
    end
    return c
end

function c_switch(f)
    local s = 'const II_Cmd_t* ii_find_command( uint8_t address, uint8_t cmd ){\n'
           .. '\tswitch( address  ){\n'
    for _,f in ipairs(files) do
        s = s .. '\t\tcase ' .. f.i2c_address .. ':\n'
              .. '\t\t\tswitch( cmd ){\n'
        local ix = 0
        for _,v in ipairs( f.commands ) do
            -- setters
            s = s .. '\t\t\t\tcase ' .. v.cmd .. ': return &'
                  .. f.lua_name .. ix .. ';\n'
            ix = ix + 1
        end
        for _,v in ipairs( f.commands ) do
            if v.get == true then
                -- implicit getter
                s = s .. '\t\t\t\tcase ' .. (v.cmd + get_offset) .. ': return &'
                      .. f.lua_name .. ix .. ';\n'
                ix = ix + 1
            end
        end
        if f.getters ~= nil then
            for _,v in ipairs( f.getters ) do
                s = s .. '\t\t\t\tcase ' .. v.cmd .. ': return &'
                      .. f.lua_name .. ix .. ';\n'
                ix = ix + 1
            end
        end
        s = s .. '\t\t\t\tdefault: return NULL; // unknown command\n'
              .. '\t\t\t}\n'
    end
    s = s .. '\t\tdefault: return NULL; // unknown address\n'
          .. '\t}\n'
          .. '}\n'
    return s
end

-- generate a printable c-string describing the module's commands
function generate_prototypes( d )
    local prototypes = ''
    local proto_prefix = 'II.' .. d.lua_name .. '.'
    for _,v in ipairs( d.commands ) do
        local s = '"' .. proto_prefix .. v.name .. '( '
        if type(v.args[1]) == 'table' then -- more than 1 arg
            local arg_count = #(v.args)
            for i=1,arg_count do
                s = s .. v.args[i][1]
                if i ~= arg_count then s = s .. ', ' end
            end
        else -- only 1 arg
            s = s .. v.args[1]
        end
        s = s .. ' )\\n\\r"\n'
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
    local get_prefix = '"II.' .. d.lua_name .. '.get( \''

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
            s = s .. ' )\\n\\r"\n'
            getters = getters .. s
        end
    end

    -- get-only commands, or custom getters
    if d.getters ~= nil then
        for _,v in ipairs( d.getters ) do
            local s = get_prefix .. v.name .. '\''
            if v.args ~= nil then
                if type(v.args[1]) == 'table' then -- multiple args
                    for i=1,#(v.args)-1 do -- ignore last
                        s = s .. ', ' .. v.args[i][1]
                    end
                else s = s .. ', ' .. v.args[1] end
            end
            s = s .. ' )\\n\\r"\n'
            getters = getters .. s
        end
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

    local events = '"II.' .. d.lua_name .. '.event = function( e, data )\\n\\r"\n'

    local overloaded = has_get_cmd(d)
    if overloaded then
        local heading = true
        for _,v in ipairs( d.commands ) do
            if v.get == true then
                if heading then
                    events = events .. '"\tif e == \'' .. v.name .. '\' then\\n\\r"\n'
                           .. '"\t\t-- handle ' .. v.name .. ' param here\\n\\r"\n'
                    heading = false
                else
                    events = events .. '"\telseif e == \'' .. v.name .. '\' then\\n\\r"\n'
                end
            end
        end
    else
        events = events .. '"\tif e == \'' .. d.getters[1].name .. '\' then\\n\\r"\n'
               .. '"\t\t-- handle ' .. d.getters[1].name .. ' param here\\n\\r"\n'
    end

    if d.getters ~= nil then
        for i=(overloaded and 1 or 2),#(d.getters) do
            events = events .. '"\telseif e == \'' .. d.getters[i].name .. '\' then\\n\\r"\n'
        end
    end
    events = events .. '"\tend\\n\\r"\n'
                    .. '"end\\n\\r"\n'
    return events
end

function make_help(f)
    local h = '"-- commands\\n\\r"\n'
            .. generate_prototypes(f) .. '"\\n\\r"\n'
    if has_getters(f) then
        h = h .. '"-- request params\\n\\r"\n'
              .. generate_getters(f) .. '"\\n\\r"\n'
              .. '"-- then receive\\n\\r"\n'
              .. generate_events(f)
    end
    return h
end

function make_c(f)
    local c = '// THIS FILE IS AUTOGENERATED //\n'
            .. '// DO NOT EDIT THIS MANUALLY //\n\n'
            .. '#pragma once\n\n'

    c = c .. 'typedef enum{ II_void\n'
          .. '            , II_u8\n'
          .. '            , II_s8\n'
          .. '            , II_u16\n'
          .. '            , II_s16\n'
          .. '            , II_s16V\n'
          .. '            , II_float   // 32bit (for crow to crow comm\'n)\n'
          .. '} II_Type_t;\n'
          .. '\n'
          .. 'typedef struct{\n'
          .. '    uint8_t cmd;\n'
          .. '    uint8_t args;\n'
          .. '    II_Type_t return_type;\n'
          .. '    II_Type_t argtype[];\n'
          .. '} II_Cmd_t;\n\n'
          .. 'const char* ii_module_list =\n'
          .. '\"--- ii: supported modules\\n\\r"\n'
    for _,f in ipairs(files) do
        c = c .. '"' .. f.lua_name .. '\\t-- ' .. f.module_name .. '\\n\\r"\n'
    end
    c = c .. '"\\n\\r"\n'
          .. '"--- See a module\'s commands with \'II.<module>.help()\'\\n\\r"\n'
          .. '"II.jf.help()\\n\\r"\n'
          .. ';\n'
    for _,f in ipairs(files) do
        c = c .. c_cmds(f)
    end
    c = c .. c_switch(files)
    for _,f in ipairs(files) do
        c = c .. 'const char* ' .. f.lua_name .. '_help=\n' .. make_help(f) .. ';\n'
    end
    c = c .. 'const char* ii_list_commands( uint8_t address )\n'
          .. '{\n'
          .. '\tswitch( address ){\n'
    for _,f in ipairs(files) do
        c = c .. '\t\tcase ' .. f.i2c_address .. ': return ' .. f.lua_name .. '_help;\n'
    end
    c = c .. '\t\tdefault: return "module not found\\n\\r";\n'
          .. '\t}\n'
          .. '}\n'
    return c
end

local in_file_dir = arg[1]
local out_file = arg[2]

do
    dir = io.popen('/bin/ls ' .. in_file_dir)
    files = {}
    for filename in dir:lines() do
        table.insert(files, dofile('lua/ii/' .. filename))
    end

    c = io.open( out_file, 'w' )
    c:write(make_c(files))
    c:close()
end

-- example usage:
-- lua util/ii_c_layer.lua lua/ii/ build/ii_c_layer.h
