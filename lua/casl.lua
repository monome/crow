local Casl = {}

function Casl.new()
    local c = {}
    c.dyn_count = 0  -- count of dynamic objects
    c.dyn_names = {} -- dynamic name -> index lookup
    c.dyn_data  = {} -- dynamic data                    TODO must be backed by C-userdata
    c.dyn       = {} -- dynamic update functions
    c.AST       = {}
    setmetatable(c, Casl)
    return c
end

function Casl:describe(d)
    casl_describe(d)
end


local function tablen(t)
    local c = 0
    for _ in pairs(t) do c = c + 1 end
    return c
end



----- IS THIS ACTUALLY JUST Casl:describe()?
--- compiler functions
-- the received table of tables contains {compiler,data} pairs
-- every compiler will return a value/literal-table to be inserted in the AST
function Casl:compile(d)
    self.AST = {}
    local function _compile(self,d)
        for k,v in ipairs(d) do
            if type(v) == 'table' then
                table.insert( self.AST, 'enter' )
                _compile(self,v) -- recursively handle nested tables
                table.insert( self.AST, 'exit' )
            else -- assume type(v)=='function'
                table.insert( self.AST
                            , v(self, d[2]) ) -- compile into self
                -- TODO recursively compile d[2] (ie behavioural-args can be nested behaviours)
                return -- d[2] is just the arg to this compiler
            end
        end
    end
    _compile(self,d)
end



function Casl:dyn_compiler(dyn)
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

-- return a table with the compiler and table
function dyn(d)
    return {Casl.dyn_compiler, d}
end






setmetatable(Casl, Casl)

return Casl
