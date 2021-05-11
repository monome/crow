function to(v,t,s)
    -- return function(self)
        return v
    -- end
end

function T(s, v, exec)
    local vv, ee = s()
    if ee then
        if exec == ee then return
        else
            print('exec: ' .. exec .. ' ~= ' .. ee)
            assert(false)
        end
    elseif v == vv then
        if exec then
            if exec == ee then return
            else
                print('exec: ' .. exec .. ' ~= ' .. ee)
                assert(false)
            end
        else return end
    else
        print('fail: ' .. v .. ' ~= ' .. vv)
        assert(false)
    end
end

sequins = dofile("lua/sequins_cond.lua")
aslsequins = dofile("lua/aslsequins.lua")


-- loop construct
sASL = loop{ to(1), to(3) } -- loop{} is implicit
T(sASL, 1)
T(sASL, 3)
T(sASL, 1)

-- TODO once construct will be default behaviour for a raw table
sASL = once{ to(1) }
-- for i=1,6 do print(i);print('_',sASL()) end
T(sASL, 1)
T(sASL, {}, 'skip')
T(sASL, {}, 'skip')

-- nested constructs
sASL = loop{ to(1), once{ to(3) } }
T(sASL, 1)
T(sASL, 3)
T(sASL, 1)
print(sASL())
print(sASL())
T(sASL, 1)

-- times construct
sASL = times(2, { to(1) })
T(sASL, 1)
T(sASL, 1)
T(sASL, {}, 'skip')


-- held & lock
-- TODO force :next() if changing hold/lock state to false (ie we need to step to the next valid elem if force-leaving)
sASL = once{ held{ to(1) }
           , to(2)
           }
aslsequins.sethold(true)
T(sASL,1,'again')
T(sASL,1,'again')
aslsequins.sethold(false)
T(sASL,2)
T(sASL, {}, 'skip')


sASL = once{ held{ to(1) }
           , to(2)
           }
aslsequins.sethold(false)
T(sASL,2)
T(sASL, {}, 'skip')
