local Output = {}

function Output.new( chan )
    local o = { channel = chan
              , level   = 1.0
              , rate    = 1.0
              , shape   = 'linear'
              , asl     = Asl.new( chan )
--              , trig    = { asl      = Asl.new(k)
--                          , polarity = 1
--                          , time     = 1
--                          , level    = 5
--                          }
              }
    o.asl.action = lfo( function() return o.rate  end
                      , function() return o.shape end
                      , function() return o.level end
                      )
    return o
end

return Output
