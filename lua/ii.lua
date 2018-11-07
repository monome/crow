--- Crow i2c library for lua frontend
--

local ii = {}

function ii.setup()
    -- sets stm32 i2c rx address?
    --
    -- TODO
    -- big idea is that this fn could take the name of the destination
    -- module ('Just Friends' or 'ansible') and *generate* the stub
    -- fns for that module.
    -- also returning the name of that module's functions in the repl.
    -- the user could then use custom fns per destination 
    --
    -- strike this as the sender often wants to send to multiple dest's
    -- and thus it makes no sense to only have a small subset avail
end

function ii.send()
    -- consider whether the TT commands could be aliased to something
    -- more in the lua/crow style like carp lets u rename C libs.
    -- 
    -- there could be the option of JT.BPM but also justtype.set_tempo
end

-- should keep a reference of i2c addresses to accompanying fns
-- might be an issue to have *all* the i2c address combos have
-- stub functions. (bc i think lua program is loaded into ram,
-- and having 500 extra fns probably uses a good chunk)
-- instead, the diff modules are diff source files which can
-- be loaded with dofile() when the i2c address is set
function ii.callback()
    -- this actually needs to be a clear subset of II commands
    -- to fit into the i2c reception model. crow can only accept,
    -- or even expect, messages to be sent in the typical manner
    -- for TT.
    -- what is the set of commands that we must accept?
    --
    -- CW.OUT chan value slew -> Maybe getter
    -- CW.IN chan -> input_val
    -- CW.CMD which value (triggers a callback in lua for the
    --      user to customize specific meaning
    --      if user wants more than 1 arg, they can redefine
    --      CW.OUT
    -- CW.RQ should query and return a value
end

print 'ii loaded'

return ii
