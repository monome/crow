_t = dofile("util/test.lua")
Asl = dofile("lua/asl.lua")

asl = {}

-- setup mocks
to_handler = function( id )
    asl:step()
end

last_to = { id = 0
              , d  = 0
              , t  = 0
              , s  = '' }
function LL_toward( id, d, t, s )
    if type(d) == 'function' then d = d() end
    if type(t) == 'function' then t = t() end
    if type(s) == 'function' then s = s() end
    last_to.id, last_to.d, last_to.t, last_to.s = id,d,t,s
    --print('id: '..id,'\tto '..d,'\tin time: '..t,'\twith shape: '..s)
end

function set_last_to(curve) last_to.s = curve end

function get_last_to()
    return {last_to.id, last_to.d, last_to.t, last_to.s}
end

function LL_get_state( id )
    return last_to.d
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
    _t.run( 'Standalone to'
          , function(id,d,t,s)
                local sl = Asl.new(id)
                sl.action = to( d,t,s )
                sl:action()
                return get_last_to()
            end
          , {{1,2,2,'linear'}, {1,2,2,'linear'}}
          )
    _t.run( 'Toward Sequence'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { to( 1,1,'linear' )
                            , to( 3,3,'expo' )
                            }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'expo'}}
          , {2, {1,3,3,'expo'}}
          )
    _t.run( 'to sequence with strings'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { to( 1,1,'linear' )
                            , 'a tag'
                            , to( 3,3,'expo' )
                            , 'another tag'
                            }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'expo'}}
          , {2, {1,3,3,'expo'}}
          )
    _t.run( 'sequence with instant actions'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { to( 1,1,'linear' )
                            , to{ now = 4 }
                            , to( 3,3,'expo' )
                            }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'expo'}}
          , {2, {1,3,3,'expo'}}
          )
    _t.run( 'sequence with restart after finish'
          , function(count)
                local sl = Asl.new(1)
                sl.action = { to( 1,1,'linear' )
                            , to( 3,3,'log' )
                            }
                sl:action()
                for i=1,2 do sl:step() end
                sl:action('restart')
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,3,3,'log'}}
          , {2, {1,3,3,'log'}}
          )

    _t.run( 'loop{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ to( 1,1 )
                                , to( 2,2 )
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,2,2,'linear'}}
          , {2, {1,1,1,'linear'}}
          )

    _t.run( 'nested loop{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ loop{ to( 1, 1 )
                                      , to( 2, 1 )
                                      }
                                , loop{ to( 3, 1 )
                                      , to( 4, 1 )
                                      }
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
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
                                    , { to( 3,3 )
                                      , to( 4,4 )
                                      }
                                    )
                            , to( 2,2 )
                            }
                sl:action()
                return get_last_to()
            end
          , {true , {1,3,3,'linear'}}
          , {false, {1,2,2,'linear'}}
          )

    _t.run( 'asl_wrap{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = asl_wrap( function() set_last_to( 'before' ) end
                                    , { to( 3,3 )
                                      , to( 4,4 )
                                      }
                                    , function() set_last_to( 'after' ) end
                                    )
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,3,3,'linear'}}
          , {1, {1,4,4,'linear'}}
          , {2, {1,4,4,'after'}}
          )

    _t.run( 'nested asl_wrap{}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ asl_wrap( function() set_last_to( 'before' ) end
                                          , { to( 3,3 )
                                            , to( 4,4 )
                                            }
                                          , function() set_last_to( 'after' ) end
                                          )
                                , to( 5,5 )
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
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
                                   , { to( 3,3 ) }
                                   )
                            , to( 5,5 )
                            }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
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
                                       , { to( 3,3 ) }
                                       )
                                , to( 5,5 )
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,3,3,'linear'}}
          , {1, {1,3,3,'linear'}}
          , {2, {1,5,5,'linear'}}
          , {3, {1,3,3,'linear'}}
          , {4, {1,3,3,'linear'}}
          , {5, {1,5,5,'linear'}}
          )

    _t.run( 'held{}'
          , function(...)
                local t = {...}
                local sl = Asl.new(1)
                sl.action = { held{ to( 3,3 )
                                  , to( 2,2 )
                                  }
                            , to( 5,5 )
                            }
                for i=1,#t do sl:action(t[i]) end
                return get_last_to()
            end
          , {{''}                 , {1,3,3,'linear'}}
          , {{'','step'}          , {1,2,2,'linear'}}
          , {{'','step','step'}   , {1,2,2,'linear'}}
          , {{'','restart'}       , {1,3,3,'linear'}}
          , {{'release'}          , {1,5,5,'linear'}}
          , {{'','release'}       , {1,5,5,'linear'}}
          , {{true,false}         , {1,5,5,'linear'}}
          , {{'release','step'}   , {1,5,5,'linear'}}
          )

    _t.run( 'held{} with leading to'
          , function(...)
                local t = {...}
                local sl = Asl.new(1)
                sl.action = { to( 4,4 )
                            , held{ to( 3,3 )
                                  , to( 2,2 )
                                  }
                            , to( 5,5 )
                            }
                for i=1,#t do sl:action(t[i]) end
                return get_last_to()
            end
          , {{''}                 , {1,4,4,'linear'}}
          , {{'','step'}          , {1,3,3,'linear'}}
          , {{'','step','step'}   , {1,2,2,'linear'}}
          , {{'','restart'}       , {1,4,4,'linear'}}
          , {{'release'}          , {1,4,4,'linear'}}
          , {{'release','step'}   , {1,5,5,'linear'}}
          , {{'','release'}       , {1,5,5,'linear'}}
          , {{true,true,false}    , {1,5,5,'linear'}}
          )

    _t.run( 'held{loop{}}'
          , function(...)
                local t = {...}
                local sl = Asl.new(1)
                sl.action = { held{ loop{ to( 3,3 )
                                        , to( 2,2 )
                                        }
                                  }
                            , to( 5,5 )
                            }
                for i=1,#t do sl:action(t[i]) end
                return get_last_to()
            end
          , {{true}                 , {1,3,3,'linear'}}
          , {{true,'step'}          , {1,2,2,'linear'}}
          , {{true,'step','step'}   , {1,3,3,'linear'}}
          , {{false}                , {1,5,5,'linear'}}
          , {{true,false}           , {1,5,5,'linear'}}
          )

    _t.run( 'loop{held{}}'
          , function(...)
                local t = {...}
                local sl = Asl.new(1)
                sl.action = loop{ held{ to( 3,3 )
                                      , to( 2,2 )
                                      }
                                , to( 5,5 )
                                }
                for i=1,#t do sl:action(t[i]) end
                return get_last_to()
            end
          , {{true}                 , {1,3,3,'linear'}}
          , {{true,'step'}          , {1,2,2,'linear'}}
          , {{true,'step','step'}   , {1,2,2,'linear'}}
          , {{false}                , {1,5,5,'linear'}}
          , {{false,false}          , {1,5,5,'linear'}}
          , {{false,true}           , {1,3,3,'linear'}}
          )

    _t.run( '{held{},to}'
          , function(...)
                local t = {...}
                local sl = Asl.new(1)
                sl.action = { held{ to( 3,3 )
                                  , to( 2,2 )
                                  }
                            , to( 5,5 )
                            }
                for i=1,#t do sl:action(t[i]) end
                return get_last_to()
            end
          , {{true}                 , {1,3,3,'linear'}}
          , {{true,true}            , {1,2,2,'linear'}}
          , {{false}                , {1,5,5,'linear'}}
          , {{true,false}           , {1,5,5,'linear'}}
          , {{true,true,false}      , {1,5,5,'linear'}}
          , {{true,'nil',false}     , {1,5,5,'linear'}}
          , {{false,'nil'}          , {1,3,3,'linear'}}
          )

    _t.run( 'loop{{to,to},to}'
          , function(count)
                local sl = Asl.new(1)
                sl.action = loop{ { to( 1,1 )
                                  , to( 2,2 )
                                  }
                                , to( 3,3 )
                                }
                sl:action()
                for i=1,count do sl:step() end
                return get_last_to()
            end
          , {0, {1,1,1,'linear'}}
          , {1, {1,2,2,'linear'}}
          , {2, {1,3,3,'linear'}}
          , {3, {1,1,1,'linear'}}
          )



--    _t.run( 'weave'
--          , function(count)
--                local sl = Asl.new(1)
--                sl.action = weave{ loop{ to( 1, 1 )
--                                       , to( 2, 1 )
--                                       }
--                                 , loop{ to( 3, 1 )
--                                       , to( 4, 1 )
--                                       }
--                                 }
--                sl:action()
--                for i=1,count do sl:step() end
--                return get_last_to()
--            end
--          , {0, {1,1,1,'linear'}}
--          , {1, {1,3,1,'linear'}}
--          , {2, {1,2,1,'linear'}}
--          , {3, {1,4,1,'linear'}}
--          , {4, {1,1,1,'linear'}}
--          )
--

---- TODO add test for lock
end

run_tests()

---- weird use-case for arbitrary code-execution in asl construct
--function sync(asl)
--    for i=2,4 do
--        output[i]:action()
--    end
--end
--
--output[1].action = loop{ sync
--                       , to(1,1)
--                       , to(2,2)
--                       }
--
