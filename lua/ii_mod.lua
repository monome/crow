function c_ii_cmd_lookup(mod_name, mod_cmd)
    -- just a big sequential string lookup
    -- start with a naive iterating solution
    -- can improve with smarter search later
    if ix == "help" then
        elseif ix == "event" then
        elseif ix == "get" then

    -- below here is generated list
        elseif ix == "trigger" then
        end
    return _
end

function c_ii_field_lookup(mod_name)
    -- this should be able to be baked in with a C reference
    -- get it at initialization from .new
    -- thus it can just be a direct field lookup
    return _
end

function c_ii_index(name, ix)
    -- must return a lua object, probably a function
    -- i think this can be a C function
    -- going to be a lot of C fns, but all autogenerated
    -- probably need a helper function for clean code

        -- TODO TODO TODO
        -- because lua is non-reentrant and we know that
        -- all calls are sequential. we can just return a
        -- generic param gathering function, meanwhile setting
        -- the address / name / ix in C, knowing that it will
        -- be called immediately with the parameters.

        -- it's like a closure where we pre-assign the upvals
        -- in order to preserve mem of allocating a whole
        -- closure in lua. we can justify it because our arch
        -- disallows any other lua code to run before it
        -- completes.

    -- it's kinda ugly & horrible code practice, but it
    -- would be the fastest & lowest overhead way to a
    -- working solution. plus avoids the massive code-bloat
    -- of separate functions for every cmd

    -- no need to check, as we generated this list!
    ii_field = c_ii_field_lookup(name)
    return ii_cmd = c_ii_cmd_lookup(ii_field, ix)
end

c_addr = 120
c_addrs = {120, 121}
function c_ii_setaddress(name, ix)
    -- note: indexing may not be required & can just be skipped
    c_addr = c_addrs[ix]
    if not _addr then
        _addr=_addrs[1]
    end
    return self
end
