get_offset = 0x80

-- allows descriptor to use shorthand without stringifying
void = 'ii_void'
s8 = 'ii_s8'
u8 = 'ii_u8'
s16 = 'ii_s16'
s16V = 'ii_s16V'
u16 = 'ii_u16'
float = 'ii_float'

function c_cmds(f)
    local c = ''

    local function make_a_cmd( lua_name, ix, cmd, args, retval )
        local s = 'const ii_Cmd_t ' .. lua_name .. ix .. ' = {' .. cmd .. ','
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

function c_unpickle(files)
    local s = 'static void ii_unpickle( uint8_t* address\n'
           .. '                       , uint8_t* command\n'
           .. '                       , uint8_t* data\n'
           .. '                       ){\n'
           .. '\tswitch( *address ){\n'
    for _,f in ipairs(files) do
        if f.unpickle then
            s = s .. '\t\tcase ' .. f.i2c_address .. ':{ // '.. f.module_name..'\n'
                  .. '\t\t\t' .. string.gsub(f.unpickle,"\n","\n\t\t\t")
                  .. '\n\t\t\tbreak;}\n'
        end
    end
    s = s .. '\t\tdefault: return; // no custom pickler\n'
          .. '\t}\n'
          .. '}\n\n'
    return s
end

function c_pickle(files)
    local s = 'static void ii_pickle( uint8_t* address\n'
           .. '                     , uint8_t* data\n'
           .. '                     , uint8_t* byte_count\n'
           .. '                     ){\n'
           .. '\tswitch( *address ){\n'
    for _,f in ipairs(files) do
        if f.pickle then
            s = s .. '\t\tcase ' .. f.i2c_address .. ':{ // '.. f.module_name..'\n'
                  .. '\t\t\t' .. string.gsub(f.pickle,"\n","\n\t\t\t")
                  .. '\n\t\t\tbreak;}\n'
        end
    end
    s = s .. '\t\tdefault: return; // no custom pickler\n'
          .. '\t}\n'
          .. '}\n\n'
    return s
end

function c_switch(files)
    local s = 'const ii_Cmd_t* ii_find_command( uint8_t address, uint8_t cmd ){\n'
           .. '\tswitch( address ){\n'
    for _,f in ipairs(files) do
        if type(f.i2c_address) == 'table' then
            for k,v in ipairs(f.i2c_address) do
                s = s .. '\t\tcase ' .. v .. ':'
                if k==1 then s = s .. ' // '.. f.module_name end
                s = s .. '\n'
            end
        else
            s = s .. '\t\tcase ' .. f.i2c_address .. ': // '.. f.module_name..'\n'
        end
        s = s .. '\t\t\tswitch( cmd ){\n'
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
          .. '}\n\n'
    return s
end

-- generate a printable c-string describing the module's commands
function generate_prototypes( d )
    local prototypes = ''
    local proto_prefix = 'ii.' .. d.lua_name .. '.'
    for _,v in ipairs( d.commands ) do
        local s = '"' .. proto_prefix .. v.name .. '( '
        if not v.args then -- no args
            s = s .. void
        elseif type(v.args[1]) == 'table' then -- more than 1 arg
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
    local get_prefix = '"ii.' .. d.lua_name .. '.get( \''

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

    local events = '"ii.' .. d.lua_name .. '.event = function( e, data )\\n\\r"\n'

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

function make_mhelp(f)
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

-- TODO: can use a multi-line string?
types_and_commands =
       'typedef enum{ ii_void\n'
    .. '            , ii_u8\n'
    .. '            , ii_s8\n'
    .. '            , ii_u16\n'
    .. '            , ii_s16\n'
    .. '            , ii_s16V\n'
    .. '            , ii_float   // 32bit (for crow to crow comm\'n)\n'
    .. '} ii_Type_t;\n'
    .. '\n'
    .. 'typedef struct{\n'
    .. '    uint8_t cmd;\n'
    .. '    uint8_t args;\n'
    .. '    ii_Type_t return_type;\n'
    .. '    ii_Type_t argtype[];\n'
    .. '} ii_Cmd_t;\n\n'

make_header = '// THIS FILE IS AUTOGENERATED //\n'
           .. '// DO NOT EDIT THIS MANUALLY //\n\n'
           .. '#pragma once\n\n'

function make_help(files)
    local c = 'const char* ii_module_list =\n'
           .. '\"--- ii: supported modules\\n\\r"\n'
    for _,f in ipairs(files) do
        c = c .. '"' .. f.lua_name .. '\\t-- ' .. f.module_name .. '\\n\\r"\n'
    end
    return c .. '"\\n\\r"\n'
             .. '"--- See a module\'s commands with \'ii.<module>.help()\'\\n\\r"\n'
             .. '"ii.jf.help()\\n\\r"\n'
             .. ';\n\n'
end

function make_commandlist(files)
    local c = ''
    for _,f in ipairs(files) do
        c = c .. c_cmds(f)
    end
    return c .. '\n'
end

function make_helpstrings(files)
    local c = ''
    for _,f in ipairs(files) do
        c = c .. 'const char* ' .. f.lua_name .. '_help=\n' .. make_mhelp(f) .. ';\n\n'
    end
    return c
end

function make_helpget(files)
    local c = 'const char* ii_list_commands( uint8_t address )\n'
           .. '{\n'
           .. '\tswitch( address ){\n'
    for _,f in ipairs(files) do
        local ii = f.i2c_address
        if type(ii) == 'table' then ii = ii[1] end
        c = c .. '\t\tcase ' .. ii .. ': return ' .. f.lua_name .. '_help;\n'
    end
    return c .. '\t\tdefault: return "module not found\\n\\r";\n'
             .. '\t}\n'
             .. '}\n\n'
end




function make_c(files)
    return make_header
        .. types_and_commands
        .. make_help(files)
        .. make_commandlist(files)
        .. c_switch(files)
        .. make_helpstrings(files)
        .. make_helpget(files)
        .. c_pickle(files)
        .. c_unpickle(files)
end

local in_file_dir = arg[1]
local out_file = arg[2]

do
    local dir = io.popen('/bin/ls ' .. in_file_dir)
    local files = {}
    for filename in dir:lines() do
        table.insert(files, dofile('lua/ii/' .. filename))
    end

    local c = io.open( out_file, 'w' )
    c:write(make_c(files))
    c:close()
end

-- example usage:
-- lua util/ii_c_layer.lua lua/ii/ build/ii_c_layer.h
