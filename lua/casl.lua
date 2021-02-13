local Casl = {}

-- global for now
dyn_names = {} -- dynamic name -> index lookup
dyn_data  = {} -- dynamic data                    TODO must be backed by C-userdata
Casl.dyn = {} -- dynamic update functions

function Casl.new(id)
    local c = {}
    c.id = id or 1 -- default to channel 1
    -- c.dyn_names = {} -- dynamic name -> index lookup
    -- c.dyn_data  = {} -- dynamic data                    TODO must be backed by C-userdata
    c.dyn       = Casl.dyn -- {} -- dynamic update functions -- just link to the global version
    setmetatable(c, Casl)
    return c
end

function Casl:describe(d)
    -- TODO need to add a compiler pass to link dyn{}s to self etc
    casl_describe(self.id, d) -- call to C
end

function Casl:action()
    casl_action(self.id)
end

function cto(volts, time, shape) -- rename to 'to'
    return {'TO', volts or 0.0, time or 1.0, shape or 'linear'}
end

function cloop(t)
    table.insert(t,{'RECUR'})
    return t
end

-- register named dynamic for updating
-- each name is given an index which maps directly to a C datastructure
-- creates an update-function in the self.dyn namespace
-- currently just global in casl. needs a compiler pass to link to self
function Casl.dyn_compiler(dyn) -- dyn -> {name=default}
    local ix = 0
    for k,v in pairs(dyn) do
        if not dyn_names[k] then -- check if it's a new entry
            dyn_names[k] = casl_defdynamic(1) -- self.id
        end
        ix = dyn_names[k]
        Casl.dyn[k] = function(v) -- map name to index
            casl_setdynamic(1,ix,v)
        end
        Casl.dyn[k](v) -- set the default
    end
    return ix -- return c-space index
end


--- global functions

-- return a table with the compiler and table
function cdyn(d)
    return {'DYN', Casl.dyn_compiler(d) } -- returns C ix of dynamic
end


Casl.__index = function(self, ix)
    if     ix == 'describe' then return Casl.describe
    elseif ix == 'action' then return Casl.action
    elseif ix == 'compile' then return Casl.compile
    end
end

setmetatable(Casl, Casl)

return Casl
