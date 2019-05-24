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
    return 1.0
end

function run_tests()
    -- Asl.new
    _t.type( {{Asl.new()}   , 'table' } -- should cause warning
           , {{Asl.new(1)}  , 'table' }
           , {{Asl.new(99)} , 'table' }
           )
    _t.run( function(id) return Asl.new(id).id end
          , {2  , 2  }
          , {99 , 99 }
          )
    _t.run( function(member) return Asl.new(1)[member] end
          , {'id'      , 1}
          --, {'co'      , {}}
          , {'hold'    , false}
          , {'in_hold' , false}
          , {'locked'  , false}
          )

    -- Asl:init
    _t.run( function(member) return Asl.init( Asl.new(1) )[member] end
          --, {'co'      , {}}
          , {'hold'    , false}
          , {'in_hold' , false}
          , {'locked'  , false}
          )

    -- typecheck public methods to ensure inheritance
    _t.type( {{Asl.new(1).init}   , 'function' }
           , {{Asl.new(1).step}   , 'function' }
           , {{Asl.new(1).action} , 'function' }
           )

    -- typecheck remaining fns before full test below
    _t.type( {toward( 1,1,'linear' )                         , 'thread' }
           , {asl_if( function() return true end, {} )       , 'thread' }
           , {asl_wrap( function() end, {}, function() end ) , 'thread' }
           , {loop{}      , 'thread'}
           , {lock{}      , 'thread'}
           , {held{}      , 'thread'}
           , {times(0,{}) , 'thread'}
           )

    -- coroutine
    _t.run( function(id,d,t,s)
                local sl = Asl.new(id)
                sl.action = toward( d,t,s )
                sl:action()
                return get_last_toward()
            end
          , {{1,2,2,'linear'}, {1,2,2,'linear'}}
          )

    -- sequence
    _t.run( function(count)
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

    -- sequence with instant actions
    _t.run( function(count)
                local sl = Asl.new(1)
                sl.action = { toward( 1,1,'linear' )
                            , toward{ ['here'] = 4 }
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

    -- sequence with restart after finish
    _t.run( function(count)
                local sl = Asl.new(1)
                sl.action = { toward( 1,1,'linear' )
                            , toward( 3,3,'expo' )
                            }
                sl:action()
                for i=1,2 do sl:step() end
                sl:action()
                for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'expo'}}
          , {2, {1,3,3,'expo'}}
          )

    -- loop{}
    _t.run( function(count)
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
          , {2, {1,1,1,'linear'}}
          )

    -- nested loop{}
    _t.run( function(count)
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

    -- asl_if{}
    _t.run( function(bool)
                local sl = Asl.new(1)
                sl.action = { asl_if( function(self) return bool end
                                    , { toward( 3,3 )
                                      , toward( 4,4 )
                                      }
                                    )
                            , toward( 2,2 )
                            }
                sl:action()
                --for i=1,count do sl:step() end
                return get_last_toward()
            end
          , {true , {1,3,3,'linear'}}
          , {false, {1,2,2,'linear'}}
          )

    -- asl_wrap{}
    _t.run( function(count)
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

    -- nested asl_wrap{}
    _t.run( function(count)
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

    -- times{}
    _t.run( function(count)
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
          )

    -- nested times{}
    _t.run( function(count)
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

    -- weave{}
    _t.run( function(count)
                local sl = Asl.new(1)
                sl.action = weave{ loop{ toward( 1, 1 )
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
          , {1, {1,3,1,'linear'}}
          , {2, {1,2,1,'linear'}}
          , {3, {1,4,1,'linear'}}
          , {4, {1,1,1,'linear'}}
          )


---- TODO add test for held & lock
end

run_tests()
