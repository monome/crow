local function strip_comments(line)
    local s,e = line:find("%-%-")
    if s == 1 then -- whole line comment
        line = ''
    elseif s then -- partial comment
        line = line:sub(1,s-1)
    end
    return line
end

local function strip_whitespace(line)
    local function whole_line(l)
        return l:find("%S") and l or ''
    end

    local function leading(l)
        local s = l:find("%S")
        if s and s > 1 then
            l = l:sub(s)
        end
        return l
    end

    local function trailing(l)
        local e = l:reverse():find("%S")
        if e and e > 1 then
            l = l:sub(1,-e)
        end
        return l
    end

    if line then -- make sure line has content
        line = whole_line( line )
        line = leading( line )
        line = trailing( line )
        return line
    else return nil end
end

local function writeline(src, dst)
  local nextline = src:read()
  if not nextline then return end -- recursive base case

  nextline = strip_comments( nextline )
  nextline = strip_whitespace( nextline )

  if nextline and nextline ~= '' then
    dst:write("\"" .. nextline .. "\\n\"\n\t")
  end

  return writeline(src, dst) -- recurse through file
end

do
  local filename = arg[1]
  do
    local f = io.open((filename .. ".h"), "w")
    f:write("#pragma once\n\nchar " .. string.gsub(string.sub(filename, 1, -5), "/", "_") .. "[]=")
    writeline(assert(io.open(filename, "r")), f)
    f:write(";")
    return f:close()
  end
end
