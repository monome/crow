--- Tests
-- dead simple testing library

-- example usage:
-- _t = dofile"test.lua"
-- _t.run( math.max        -- function to test
--       , {{3, 4}, 4}     -- test case: arguments (opt. as table), expected result
--       , {{5, 2}, 3}     -- another test case
--       )

local Test = {}

-- this is really Test.equality
function Test.run( name, fn, ... )
    local test_cases = {...}
    local failures = 0
    for _,test_case in ipairs( test_cases ) do
        local arguments = test_case[1]
        local expect    = test_case[2]
        local result    = {}
    -- apply function
        if type( arguments ) == 'table' then
            result = fn( table.unpack( arguments ) )
        else result = fn( arguments ) end
    -- compare results
        if type(expect) == 'table' then
            local mr_failure = 0
            for i=1,#expect do
                if expect[i] ~= result[i] then
                    mr_failure = mr_failure + 1
                end
            end
            if mr_failure ~= 0 then
                failures = failures + 1
                io.write('FAILED!'..'\texpect\t')
                for i=1,#expect do
                    io.write(expect[i]..'\t')
                end
                print()
                io.write('\tresult\t')
                for i=1,#expect do
                    io.write(result[i]..'\t')
                end
                print()
            end
        elseif expect ~= result then
            failures = failures + 1
            print('FAILED!'..'\texpect\t'..expect)
            print('\tresult\t'..result)
        end
        --else print'ok!' end
    end

-- status print
    if failures == 0 then
        print(#test_cases..' tests passed. ' .. name)
    else
        io.write(failures..' tests failed, ')
        print((#test_cases-failures)..' passed, in ' .. name)
    end
end

function Test.type( name, ... )
    Test.run( name, type, ... )
end

return Test
