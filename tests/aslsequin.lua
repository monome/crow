function to(v,t,s)
    -- return function(self)
        return v
    -- end
end

function T(s, v, exec)
    local vv, ee = s()
    if ee then
        if exec == ee then return
        else print'bad exec' end
    elseif v == vv then
        if exec then
            if exec == ee then return
            else print'bad exec' end
        else return end
    else
        -- print('fail: ' .. v .. ' ~= ' .. vv)
        assert(false)
    end
end

sequins = dofile("lua/sequins_alt.lua")
aslsequins = dofile("lua/aslsequins.lua")


--= S{to(1), to(3)}
sASL = loop{ to(1), to(3) } -- loop{} is implicit
T(sASL, 1)
T(sASL, 3)
T(sASL, 1)

sASL = once{ to(1) }
-- for i=1,6 do print(i);print('',sASL()) end
T(sASL, 1)
T(sASL, {}, 'dead')
T(sASL, {}, 'dead')


sASL = loop{ to(1), once{ to(3) } }
T(sASL, 1)
T(sASL, 3)
T(sASL, 1)
T(sASL, 1)
