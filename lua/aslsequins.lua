--- ASL in terms of sequins

-- sequins = require(sequins) -- 'sequins' is assumed to be a valid global for pointing to the sequins lib
S = sequins

local ASL = { hold_state = false }

ASL.sethold = function(b) ASL.hold_state = b end
local function is_held() return ASL.hold_state end


----------------------------------------------
--- assign action

function ASL.set_action(self, t)
    if not S.is_sequins(t) then t = once(t) end -- wrap raw table into sequins
    self.seq = t
    -- self.running = false
    -- self.locked = false
    -- self.holding = false
end


----------------------------------------------
--- directives

ASL.holdtrue = function(self) ASL.hold_state = true end
ASL.holdfalse = function(self) ASL.hold_state = false end
ASL.restart = function(self)
    -- TODO reset all sequins.action indexes
    ASL.holdtrue()
end

function ASL.do_table(self, t)
    ASL.set_action(self, t)
    ASL.restart(self)
end

local dostrtab = { start   = ASL.holdtrue
                 , restart = ASL.restart
                 , attack  = ASL.restart
                 , release = ASL.holdfalse
                 , unlock  = function(self) ASL.locked = false end
                 }
function ASL.do_string(self, s) dostrtab[s](self) end

function ASL.do_bool(self, s)
    if s then ASL.attack(self) else ASL.holdfalse(self) end
end

local dirtab = { table   = ASL.do_table
               , string  = ASL.do_string
               , boolean = ASL.do_bool
               , number  = ASL.holdtrue
               }
function ASL.directive(self, d)
    -- main entrypoint to runtime behaviour
    if d then dirtab[type(d)](self, d) end
    local ret, exec = self.seq() -- get next sequin value
    -- TODO call ret if it exists
    -- TODO handle exec
end


----------------------------------------------
--- constructs
-- alternate syntax for sequins with some ASL-context aware conditions

function loop(t) return S(t) end
function times(n, t) return S(t):all():times(n) end
    -- TODO refactor by adding a 'dead' terminal element in the table (rather than using these constructs)
function once(t) return S(t):all():times(1) end
function held(t) return S(t):condr(is_held) end
function lock(t)
    print'TODO lock'
end


return ASL
