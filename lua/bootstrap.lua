--- DEPRECATED!!
--- NOT INCLUDED IN BINARY






--- Bootstrap Lua by redefining standard library functions
-- dofile() and print() need hardware specific implementations
-- call this before any other lua code
--
-- nb: assert() seems to be broken. failed assert does nothing

-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
-- must set garbage collection faster than normal or lua VM stack overflows!
-- TODO: optimize this choice of value
-- setpause of 60 seems to give best results for very fast input[n].mode('stream')
collectgarbage('setpause', 55) --default 200
collectgarbage('setstepmul', 260) --default 200
-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

print = function(...)
    local args = {...}
    local arg_len = #args
    local printResult = args[1]
    if arg_len > 1 then
        for i=2,arg_len do
            printResult = printResult .. '\t' .. tostring(args[i])
        end
    end
    print_serial(tostring(printResult))
end

-- nb: this is basically the inverse of l2h.lua (but we don't want that at RT)
dofile = function( path )
    return c_dofile( luapath_to_cpath( path ))
end

function luapath_to_cpath( path )
    -- string manipulation: 'lua/asl.lua' -> 'lua_asl'
    -- see: l2h.lua for how lua is compiled into c-header files
    return string.gsub
            ( string.sub
                ( path
                , 1
                , string.find(path, '%.')-1
                )
            , '/'
            , '_'
            )
end

_c = dofile('lua/crowlib.lua')
crow = _c

--collectgarbage('collect')
