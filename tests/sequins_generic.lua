S.every -- conditional execution. return 'skip' if predicate evaluates false
S.count -- always executes. evaluate predicate. return 'again' if true
S.times -- conditional execution. return 'dead' if predicate evaluates false

every.act = { _exec = 'skip'
            , _pred = function(act, n) return (act.ix % n) == 0 end
            }
count.act = {}
times.act = { _exec = 'dead'
            , _pred = function(act, n) return act.ix <= 0 end
            }

function S.generic(act)
    local n = resolve(act.n)
    local up = act.up
    local retval, exec = {}, (act._exec or '')
    local is_eval = true

    if act._pred then is_eval = act._pred(act, n) end

    if is_eval then
        retval, exec = S.next(act)
        if exec == 'again' then return retval, exec end -- will recursively escape to top llayer. need to have top level handler for 'again' and whether to recur
        -- handle response
    end
    return retval, exec
end
