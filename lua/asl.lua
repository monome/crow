--- A Slope Language
-- declaratively script hardware-interactive slopes

local table = require("table")
local print = print

local Asl = {}
Asl.__index = Asl

function Asl.new(id)
    local slope = {}
    setmetatable(slope, Asl)
    if id == nil then print"asl needs id of output channel" end
    slope.id = id or 1
    slope:init()
    return slope
end

function Asl:init() -- reset to defaults
    self.program = {}     -- store the ASL program
    self.retStk  = {}     -- return stack for nested constructs
    self.pc      = 0      -- program counter within stack frame
    self.hold    = false  -- is the slope trigger currently held high
    self.in_hold = false  -- is eval currently in a held construct
    self.locked  = false  -- flag to lockout bangs during lock{}
end

-- COMPILER
-- create a table with a sequence of functions

local function exit(self)
    self.pc = self.retStk[#self.retStk]
    table.remove( self.retStk )
end

-- deferred versions of intrinsics
local function q_exit() return exit end

local function compileT( aslT )
    local x = {}
    for k, fn in ipairs(aslT) do
        table.insert( x, fn )
    end
    table.insert( x, q_exit() )
    return x
end

-- use a metamethod so we can can *assign* myAsl:action = lfo()
-- but then *call* myAsl:action(high/low) 
function Asl:action( aslT )
    self.program  = compileT( aslT )
    self.retStk   = {}
    self.hold     = false
    self.locked   = false
end


-- RUNTIME behaviour
-- relates bang() to internal AST representation

-- step through the program recursively!
-- this is stupid. we're searching the tree every step!
-- think about how to do this in a direct way

-- returns the current table context of the program counter
--> helper doASL()
local function get_frame(self)
    local frame = self.program -- the table representing the program
    for i=1, #self.retStk, 1 do
        frame = frame[self.retStk[i]]
    end
    return frame
end

local function doASL( self )
    local p = get_frame(self)                     -- find table level
    if p[self.pc] == nil then print"asl.pc == nil" return
    elseif type(p[self.pc]) == "function" then
        wait = p[self.pc](self)                  -- execute the step
        if self.pc ~= nil then
            self.pc = self.pc + 1
            if wait ~= "wait" then doASL(self) end  -- recurse for next step
        end -- else we did last exit()
    else
        table.insert( self.retStk, self.pc )
        self.pc = 1
        doASL(self)                               -- unpack table & recurse
    end
end

function Asl:callback()
    doASL(self)
end

function Asl:set_hold_state(state)
    self.hold = state
end

local function start( self )
    if self.program ~= nil then
        self.pc      = 1
        self.retStk  = {}
    end
    doASL(self)
end

local function release( self )
    self.hold = false
    if self.in_hold then
        self.in_hold = false
        exit(self)
    end
end

-- public accessors
function Asl:bang( dir )
    self.hold = dir
    if self.program ~= nil then
        if self.locked ~= true then
            if dir == true then
                start( self )
            else
                release( self )
            end
        end
    end
end





-- hw access fn
function toward( dest, time, shape )
    local d,t,s = dest or 0, time or 1, shape or "linear"
    return function(self)
        LL_toward(self.id, d,t,s)
        if t ~= 0 then
            return "wait"
        end
    end
end

-- sugared toward(). could be variable arity instead?
function now( dest )
    return toward(d,0,LINEAR)
end

function delay( time )
    local here = 0 -- can this be a metamethod call?
    return toward( here, t, LINEAR )
end





-- WRAPPING functions
function asl_if( fn_to_bool, aslT )
    local ftb = fn_to_bool
    t = compileT( aslT )
    table.insert( t
                , 1
                , function(self)
                    if ftb(self) == false then exit(self) end
                  end
                )
    return t
end

-- replace pc = 0 with tag(tagname) goto(tagname) pairs
function q_recur()
    return function(self) self.pc = 0 end
end

function asl_wrap( enter_fn, aslProgram, exit_fn )
    p = aslProgram
    table.insert( p,  1, enter_fn )
    table.insert( p, #p, exit_fn )
    return p
end





-- first layer of abstraction over VM
function loop( aslT )
    t = compileT( aslT )
    t[#t] = q_recur() -- overwrite exit() with recur()
    return t
end

-- could abstract this form into asl_wrap()
function lock( aslT )
    return asl_wrap( function(self) self.lock = true end
                   , compileT( aslT )
                   , function(self) self.lock = false end
                   )
end

function held( aslT )
    return asl_wrap( function(self) self.in_hold = true end
                   , asl_if( function(self) return self.hold end
                           , aslT
                           )
                   , function(self) self.in_hold = false end
                   )
end

function times( count, aslT )
    local n = count -- does this become an issue in the class? asl.n?
    t = asl_if( function(self) local result = n > 0
                               n = n - 1
                               return result end
              , aslT
              )
    t[#t] = q_recur()
    return t
end

return Asl
