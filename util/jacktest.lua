local stage = 1
local expectations = 
{ { in_chan = 1, out_chan = 1, expect = -5.0 } ,
{ in_chan = 1, out_chan = 2, expect = -1.0 } , 
{ in_chan = 1, out_chan = 3, expect = 3.0 } ,
{ in_chan = 1, out_chan = 4, expect = 9.0 } ,
{ in_chan = 2, out_chan = 4, expect = 7.0 } }

function test_case( e )
  print('------------------------> in[' .. e.in_chan .. '] -> out[' .. e.out_chan .. ']')
  for i=1,2 do
    if i == e.in_chan then
      input[i].mode('stream',0.1)
    else
      input[i].mode('none')
    end
  end
  for i=1,4 do
    if i == e.out_chan then
      output[i].volts = e.expect
    else
      output[i].volts = 0.0
    end
  end
end

local threshold = 0.1 -- should be much tighter tolerance
function is_in_range( value, expectation )
  if value > (expectation - threshold)
   and value < (expectation + threshold) then
    return true
  end
end

input[1].stream = function(value)
  if is_in_range(value, expectations[stage].expect) then
    print('\tok!')
    stage = stage + 1
    test_case(expectations[stage])
  else print(value)
  end
end

input[2].stream = function(value)
  if is_in_range(value, expectations[stage].expect) then
    print('TEST SUCCEEDED')
  else print(value)
  end
end

function init()
  test_case(expectations[stage])
end

