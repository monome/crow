get_offset = 0x80

function make_lualink(files)
    local ll = '// THIS FILE IS GENERATED BY util/ii_lualinker.lua.\n\n'
            .. '// DO NOT EDIT BY HAND!!!\n\n'
            .. '#pragma once\n\n'
            .. '#include \"lib/lualink.h\" // struct lua_lib_locator\n\n'
    for _,f in ipairs(files) do
        ll = ll .. '#include \"build/ii_' .. f.lua_name .. '.h\"\n'
    end
    ll = ll .. '\n'
            .. 'const struct lua_lib_locator Lua_ii_libs[] = {\n'
    for _,f in ipairs(files) do
        local name = 'build_ii_' .. f.lua_name
        ll = ll .. '\t{ \"' .. name .. '\", ' .. name .. '_lc, true, ' .. name .. '_lc_len' .. '},\n'
    end
    ll = ll .. '\t{ NULL, NULL } };\n'
    return ll
end

local in_file_dir = arg[1]
local out_file = arg[2]

do
    dir = io.popen('/bin/ls ' .. in_file_dir)
    files = {}
    for filename in dir:lines() do
        table.insert(files, dofile('lua/ii/' .. filename))
    end

    ll = io.open( out_file, 'w' )
    ll:write(make_lualink(files))
    ll:close()
end

-- example usage:
-- lua util/ii_lualinker.lua lua/ii/ build/ii_lualink.h
