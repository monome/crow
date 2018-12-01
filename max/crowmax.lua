this.outlets = 2

function scriptload()
  outlet(0, "crow")
end

local complete = false
local crow_cmd = "" -- temporary space to construct the received string
-- need to convert to string from ascii?
function from( ... )
  local t = {...}
  local len = table.remove(t)
  print(buf)
  -- a fragment of a message
  -- need a simple parser to look for start/end tokens
  if complete then
    complete = false
    eval_cmd( crow_cmd )
  end
end

function eval( str )
  -- parse for 'execute' token vs print
  if string.match( str, "^%^%^" ) then
    assert( loadstring( string.sub( str, 3 )))()
  else
    print( str )
  end
end

function get_input( channel )
  outlet(0, "get_cv("..channel..")")
end
-- continues to ->
function ret_cv( channel, value )
  outlet(1, "input", channel, value)
end