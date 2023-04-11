-- temp placing print redef here to complete bootstrap elimination
print = function(...)
    local args = {...}
    local arg_len = #args
    local printResult = args[1]
    if arg_len > 1 then
        for i=2,arg_len do
            printResult = string.format('%s\t%s', printResult, tostring(args[i]))
        end
    end
    print_serial(tostring(printResult))
end
