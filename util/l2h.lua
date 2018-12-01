local function _0_(src, dst)
  local nextline = src.read(src, "l")
  if (nextline ~= nil) then
    dst.write(dst, ("\"" .. nextline .. "\\n\"\n\t"))
    return writeline(src, dst)
  end
end
writeline = _0_
do
  local filename = arg[1]
  do
    local f = io.open((filename .. ".h"), "w")
    f.write(f, ("#pragma once\n\nchar " .. string.gsub(string.sub(filename, 1, -5), "/", "_") .. "[]="))
    writeline(assert(io.open(filename, "r")), f)
    f.write(f, ";")
    return f.close(f)
  end
end
