--- Test layer for working without a physical crow
-- this holds a crow-state description and responds to Max commands
-- to control an emulated crow

-- the byte stream sent to [serial] inlet
function list(...)
    pcall( loadstring( bytes_to_string{...}))
end

--- Crow emulation
--
-- data structure

-- API: include crowlib.lua ??
crow =
	{ cv =
		{ 3.7
        , -2.0001
        }
    }
--crow.cv[1] = 2.1
function get_cv( channel )
    to_serial("^^ret_cv(" .. channel .. "," .. crow.cv[channel] .. ")")
end

-- convert string to ascii sequence
function to_serial( string )
    ascii = { "from", 0 }
    for c in string:gmatch"." do
        table.insert(ascii, c:byte())
    end
	ascii[2] = #ascii - 2
    outlet(0, ascii)
end

function bytes_to_string( bytes )
    local string = ""
    for i=1, #bytes do
        string = string .. string.char(bytes[i])
    end
	return string
end
