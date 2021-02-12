local Casl = {}

function Casl.new(id)
    local c = {}
    c.id = id or 1 -- default to channel 1
    c.dyn_count = 0  -- count of dynamic objects
    c.dyn_names = {} -- dynamic name -> index lookup
    c.dyn_data  = {} -- dynamic data                    TODO must be backed by C-userdata
    c.dyn       = {} -- dynamic update functions
    c.AST       = {}
    setmetatable(c, Casl)
    return c
end

function Casl:describe(d)
    self.AST = {}
    Casl.compile(self,d) -- this should just build a table that we assign to self.AST
    -- self.AST = Casl.compile(self,d) -- like this!
    -- casl_describe(self.id, Casl.compile(self, d)) -- or better like this!
    casl_describe(self.id, self.AST) -- call to C. assume channel 1 for now
end

local function tablen(t)
    local c = 0
    for _ in pairs(t) do c = c + 1 end
    return c
end

local ti=table.insert

----- IS THIS ACTUALLY JUST Casl:describe()?
--- compiler functions
-- the received table of tables contains {compiler,data} pairs
-- every compiler will return a value/literal-table to be inserted in the AST
function Casl:compile(tab)
    for k,v in ipairs(tab) do
        local t = type(v)
        print('compile_type: '..t)
        if t=='table' then
            Casl.compile(self,v) -- should this just insert v?
        elseif t=='number' then
        elseif t=='string' then
        else
            ti( self.AST
              , v(self, tab[2]) ) -- compile into self
            -- TODO recursively compile tab[2] (ie behavioural-args can be nested behaviours)
            return -- tab[2] is just the arg to this compiler
        end
    end
end

function Casl:to_compiler(t)
    return {to=t}
end

-- register named dynamic for updating
-- each name is given an index which maps directly to a C datastructure
-- creates an update-function in the self.dyn namespace
function Casl:dyn_compiler(dyn) -- dyn -> {name=default}
    -- only a single k,v pair
    local ix = 0
    for k,v in pairs(dyn) do
        if not self.dyn_names[k] then -- check if it's a new entry
            self.dyn_count = self.dyn_count + 1
            self.dyn_names[k] = self.dyn_count
        end
        ix = self.dyn_names[k]
        self.dyn_data[ix] = v
        self.dyn[k] = function(v) -- map name to index
            self.dyn_data[ix] = v -- update internal value
        end
    end
    return {dyn=ix} -- return c-space index
end

Casl.__index = function(self, ix)
    if     ix == 'describe' then return Casl.describe
    elseif ix == 'compile' then return Casl.compile
    end
end

--- global functions
function cto(volts, time, shape) -- rename to 'to'
    local t={volts or 0.0, time or 1.0, shape or 'linear'} -- defaults
    return {Casl.to_compiler, t}
end

-- return a table with the compiler and table
function dyn(d)
    return {Casl.dyn_compiler, d}
end

setmetatable(Casl, Casl)

return Casl
