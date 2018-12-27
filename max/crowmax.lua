this.inlets  = 2
this.outlets = 3


--- Marshall returned serial stream
-- messages returned from serial port
local cmd_bytes  = {} -- temporary space to construct the received string
local cmd_length = -1
local cmd_rxd    = 0
function from( byte )
	table.insert( cmd_bytes, byte )
	cmd_rxd = cmd_rxd + 1
	if cmd_rxd == cmd_length then
    	eval( bytes_to_string( cmd_bytes))
		cmd_bytes  = {}
		cmd_length = -1
		cmd_rxd    = 0
    end
end

function from_length( n )
	if n ~= 0 then
		cmd_length = n
	end
end

function eval( str )
    -- parse for 'execute' token vs print
    if string.find( str, "^%^%^" ) then
		pcall( loadstring( str:sub(3)))
    else
        to_max_print( str )
    end
end


--- Request/callback pairs
-- from max
function get_cv( channel )
    tell_crow( "get_cv("..channel..")")
end
-- continues to ->
function ret_cv( channel, value )
    to_max("ret_cv", channel, value)
end


--- Helper conversion functions

function tell_crow( str )
	if string.find( str, "^%^%^" ) then -- 3char command
		outlet(0, string_to_serial( string.sub( str, 1, 3)))
	elseif string.len(str) >= 64 then -- multi-line codeblock
		-- TODO if more than 1kB, need to split into multiple messages
		outlet(0, string_to_serial( '```'))
		outlet(0, string_to_serial( str))
		outlet(0, string_to_serial( '```'))
	else -- standard code block
		outlet(0, string_to_serial( str))
	end
end

function string_to_serial( str )
    ascii = {}
    for c in str:gmatch"." do
        table.insert(ascii, c:byte())
    end
	table.insert(ascii, string.byte'\n')
    return ascii
end

function to_max( ... )
    outlet(1, ...)
end

function to_max_print( ... )
    outlet(2, ...)
end

function bytes_to_string( bytes )
    local str = ""
    for i=1, #bytes do
        str = str .. string.char(bytes[i])
    end
	return str
end