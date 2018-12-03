this.inlets  = 2
this.outlets = 2

local complete = false
local crow_cmd = "" -- temporary space to construct the received string

-- messages returned from serial port
function from( ... )
    local t = {...}
    local len = table.remove(t,1)
    -- a fragment of a message
    -- need a simple parser to look for start/end tokens
-- FIXME just assuming the whole message comes in one packet
	eval( bytes_to_string(t))
    --if complete then
    --    complete = false
    --    eval( crow_cmd )
    --end
end

function eval( str )
    -- parse for 'execute' token vs print
    if string.find( str, "^%^%^" ) then
		pcall( loadstring( str:sub(3)))
    else
        print"received print"
        print( str )
    end
end

-- from max
function get_cv( channel )
    to_serial("get_cv("..channel..")")
end
-- continues to ->
function ret_cv( channel, value )
    to_max("ret_cv", channel, value)
end


--- Helper conversion functions
-- convert string to ascii sequence
function to_serial( string )
    ascii = {}
    for c in string:gmatch"." do
        table.insert(ascii, c:byte())
    end
    outlet(0, ascii)
end

function to_max( ... )
    outlet(1, ...)
end

function bytes_to_string( bytes )
    local string = ""
    for i=1, #bytes do
        string = string .. string.char(bytes[i])
    end
	return string
end
