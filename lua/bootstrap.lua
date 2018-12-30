--- Bootstrap Lua by redefining standard library functions
-- dofile() and print() need hardware specific implementations
-- call this before any other lua code
--
-- nb: assert() seems to be broken. failed assert does nothing

-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
-- must set garbage collection faster than normal or lua VM stack overflows!
-- TODO: optimize this choice of value
collectgarbage('setpause', 100) --default 200
--collectgarbage('setstepmul', 400) --default 200
-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

print = function(...)
    local printResult = ''
    for _,v in ipairs{...} do
        printResult = printResult .. tostring(v) .. '\t'
    end
    --debug_usart(printResult) --TODO replace this with formatted print over USB?
    print_serial(printResult) --FIXME is this causing a crash?
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

print'lua bootstrapped'

_c = dofile('lua/crowlib.lua')

--collectgarbage('collect')
