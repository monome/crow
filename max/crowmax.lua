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
		if cmd_bytes[#cmd_bytes] == 13 then -- check for carriage return
			eval( bytes_to_string( cmd_bytes))
			cmd_bytes  = {}
		end
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
	if #str == 0 then return end
	local split = string.find( str, "\r" )
	local now = str:sub( 1, split)
	if string.find( now, "^%^%^") then
		pcall( loadstring( now:sub(3)))
    else
        to_max_print( now )
    end
	if split ~= nil then --in case of dropped \r
		eval( str:sub( split+1))
	end
end

--- Request/callback pairs
-- from max
function get_input( channel )
    tell_crow( "get_cv("..channel..")")
end
function get_output( channel )
    tell_crow( "get_out("..channel..")")
end
-- continues to ->
function stream( channel, value )
    to_max("stream", channel, value)
end

function change( channel, state )
    to_max("change", channel, state )
end

function output( channel, state )
    to_max("output", channel, state )
end

function midi( ... )
    to_max("midi", ...)
end

function version( ... )
    to_max("version", ...)
end

function identity( ... )
    to_max("identity", ...)
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
