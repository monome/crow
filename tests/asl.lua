_t = dofile("util/test.lua")
Asl = dofile("lua/asl.lua")

asl = {}

-- setup mocks
toward_handler = function( id )
    asl:step()
end

last_toward = { id = 0
              , d  = 0
              , t  = 0
              , s  = '' }
function LL_toward( id, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    if type(s) == 'function' then s = s() end
    last_toward.id, last_toward.d, last_toward.t, last_toward.s = id,d,t,s
    --print('id: '..id,'\ttoward '..d,'\tin time: '..t,'\twith shape: '..s)
end

function set_last_toward(curve) last_toward.s = curve end

function get_last_toward()
    return {last_toward.id, last_toward.d, last_toward.t, last_toward.s}
end

function LL_get_state( id )
    return last_toward.d
end

function run_tests()
    -- Asl.new
    _t.type( 'Asl.new'
           , {{Asl.new()}   , 'table' } -- should cause warning
           , {{Asl.new(1)}  , 'table' }
           , {{Asl.new(99)} , 'table' }
           )
    _t.run( 'Asl.new.id'
          , function(id) return Asl.new(id).id end
          , {2  , 2  }
          , {99 , 99 }
          )
    _t.run( 'Asl[member]'
          , function(member) return Asl.new(1)[member] end
          , {'id'      , 1}
          , {'hold'    , false}
          , {'in_hold' , false}
          , {'locked'  , false}
          )
    _t.run( 'Asl.init'
          , function(member) return Asl.init( Asl.new(1) )[member] end
          , {'hold'    , false}
          , {'in_hold' , false}
          , {'locked'  , false}
          )
    _t.type( 'Inheritance'
           , {{Asl.new(1).init}   , 'function' }
           , {{Asl.new(1).step}   , 'function' }
           , {{Asl.new(1).action} , 'function' }
           )
    _t.run( 'Standalone toward'
          , function(id,d,t,s)
                local sl = Asl.new(id)
                sl.action = toward( d,t,s )
                sl:action()
                return get_last_toward()
            end
          , {{1,2,2,'linear'}, {1,2,2,'linear'}}
          )
    _t.run( 'Toward Sequence'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { toward( 1,1,'linear' )
                            , toward( 3,3,'expo' )
                            }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'expo'}}
          , {2, {1,3,3,'expo'}}
          )
    _t.run( 'sequence with instant actions'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { toward( 1,1,'linear' )
                            , toward{ now = 4 }
                            , toward( 3,3,'expo' )
                            }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'expo'}}
          , {2, {1,3,3,'expo'}}
          )
    _t.run( 'sequence with restart after finish'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { toward( 1,1,'linear' )
                            , toward( 3,3,'log' )
                            }
                sl:action()
                for i=1,2 do sl:step() end
                sl:action('restart')
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'log'}}
          , {2, {1,3,3,'log'}}
          )

    _t.run( 'loop{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ toward( 1,1 )
                                , toward( 2,2 )
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,2,2,'linear'}}
          , {2, {1,1,1,'linear'}} -- fails
          )

    _t.run( 'nested loop{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ loop{ toward( 1, 1 )
                                      , toward( 2, 1 )
                                      }
                                , loop{ toward( 3, 1 )
                                      , toward( 4, 1 )
                                      }
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,2,1,'linear'}}
          , {2, {1,1,1,'linear'}}
          , {3, {1,2,1,'linear'}}
          )

    _t.run( 'asl_if{}'
          , function(bool)
                local sl = Asl.new(1)
                sl.action = { asl_if( function(self) return bool end
                                    , { toward( 3,3 )
                                      , toward( 4,4 )
                                      }
                                    )
                            , toward( 2,2 )
                            }
                sl:action()
                return get_last_toward()
            end
          , {true , {1,3,3,'linear'}}
          , {false, {1,2,2,'linear'}}
          )

    _t.run( 'asl_wrap{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = asl_wrap( function() set_last_toward( 'before' ) end
                                    , { toward( 3,3 )
                                      , toward( 4,4 )
                                      }
                                    , function() set_last_toward( 'after' ) end
                                    )
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,3,3,'linear'}}
          , {1, {1,4,4,'linear'}}
          , {2, {1,4,4,'after'}}
          )

    _t.run( 'nested asl_wrap{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ asl_wrap( function() set_last_toward( 'before' ) end
                                          , { toward( 3,3 )
                                            , toward( 4,4 )
                                            }
                                          , function() set_last_toward( 'after' ) end
                                          )
                                , toward( 5,5 )
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,3,3,'linear'}}
          , {1, {1,4,4,'linear'}}
          , {2, {1,5,5,'linear'}}
          , {3, {1,3,3,'linear'}}
          )

    _t.run( 'times{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { times( 2
                                   , { toward( 3,3 ) }
                                   )
                            , toward( 5,5 )
                            }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,3,3,'linear'}}
          , {1, {1,3,3,'linear'}}
          , {2, {1,5,5,'linear'}}
          , {3, {1,5,5,'linear'}}
          )

    _t.run( 'nested times{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ times( 2
                                       , { toward( 3,3 ) }
                                       )
                                , toward( 5,5 )
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,3,3,'linear'}}
          , {1, {1,3,3,'linear'}}
          , {2, {1,5,5,'linear'}}
          , {3, {1,3,3,'linear'}}
          , {4, {1,3,3,'linear'}}
          , {5, {1,5,5,'linear'}}
          )

    _t.run( 'TODO held{}'
          , function(...)
                local t = {...}
                local sl = Asl.new(1)
                sl.action = { held{ toward( 3,3 )
                                  , toward( 2,2 )
                                  }
                            , toward( 5,5 )
                            }
                for i=1,#t do sl:action(t[i]) end
                return get_last_toward()
            end
          , {{''}                 , {1,3,3,'linear'}}
          , {{'','step'}          , {1,2,2,'linear'}}
          , {{'','step','step'}   , {1,2,-1,'linear'}}
          , {{'','restart'}       , {1,3,3,'linear'}} -- FIXME jump back to start
          , {{'release'}          , {1,5,5,'linear'}}
          , {{'release','step'}   , {1,5,5,'linear'}}
          , {{'','release'}       , {1,5,5,'linear'}} -- FIXME jump out of A into R
          )

-- add a test as above, but held{} wraps a loop{}, and another with weave{} to make
-- sure we're successfully jumping out of the inner construct

--    _t.run( 'weave'
--          , function(count)
--                local sl = Asl.new(1)
--                sl.action = weave{ loop{ toward( 1, 1 )
--                                       , toward( 2, 1 )
--                                       }
--                                 , loop{ toward( 3, 1 )
--                                       , toward( 4, 1 )
--                                       }
--                                 }
--                sl:action()
--                for i=1,count do sl:step() end
--                return get_last_toward()
--            end
--          , {0, {1,1,1,'linear'}}
--          , {1, {1,3,1,'linear'}}
--          , {2, {1,2,1,'linear'}}
--          , {3, {1,4,1,'linear'}}
--          , {4, {1,1,1,'linear'}}
--          )
--



---- TODO add test for held & lock
end

run_tests()
