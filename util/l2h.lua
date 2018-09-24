---Lua to C-header converter
--
--just stringifys a lua file and wraps it as a char[] in a header

name = string.gsub(string.sub(arg[1],1,-5), '/', '_')
p = assert(io.open(arg[1],'r'))
f = io.open(arg[1]..'.h', 'w')
f:write('#pragma once\n\nchar '..name..'[]=')

-- also add code validation here! save having to flash and read an
-- error over uart...
-- add white-space stripping to reduce filesize (spaces v tabs!)
function writeline(src, dst)
    nextline = src:read('l')
    if nextline == nil then return end
    dst:write('\"'..nextline..'\\n\"\n\t')
    writeline(src,dst)
end

writeline(p,f)
f:write(';')
f:close()
