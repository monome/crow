--- a dynamic scaling shaper
--
-- just some notes here. should be implemented in C for performance
--
-- 2 categories of shapers are provided
--      1. relative shapers (like RC curves)
--      2. absolute shapers (quantizers)
--
-- relative shapers should be simple lookup tables with interpolation.
-- these can be relatively small tables as they are not for audio so
-- distortion isn't much of a concern.
-- 
-- these will cover the basics:
--  linear (which is just pass through)
--  square
--  log / expo
--  sine / cosine / tan
--  sec / csc
--  1/x
--
-- of note here is that all of these relative shapers for drawing a
-- line between 2 points could be applied absolutely as well, shifting
-- the baseline of the output. consider that these could be different!
--
-- 
-- the absolute shapers are interesting though as a set of divisions
-- thinking primarily in terms of absolutely dividing up the output
-- space.
-- user provides:
--  zero point to base off (could be a 'key')
--  fold point (1V becomes 1 octave, but could be eg 1v2 for buchla)
--  either | divisions (divides fold-size into n equal steps)
--         | list (divides fold-size unequally according a list)
--
-- the natural use is to provide scales for quantizing. by using the
-- list approach, one can send arbitrary pitches, so this could be
-- tonal scales directly, rather than semitones of 'divisions'.
--
-- list can also take fractions to natively handle just intonation,
-- readily accepting a scale as a list of fractions.
--
-- a weighting mechanism could be added to give a wider window to
-- preferred values.
--
-- setting divisions to a large number, then manipulating fold will
-- essentially become a bit crusher with continuously variable bits
--

local shaper = { ['zero'] = 0
               , ['fold'] = 1  --volts
               , ['divs'] = 12 --semitones
               , ['list'] = {} --empty means nil
               }
--
--
-- consider 2d waves
-- how should these relate to the Asl. 1 master, 1 slave
-- or just run 2 in parallel with diff equations.
-- eg lissajous curves
--
--

function S_absolute( samp, divs, fold )
    div_sz  = fold / divs
    bracket = math.floor(samp / fold)
    shelf   = samp % fold
    shelf1  = shelf / fold
    div     = math.floor(0.5 + (shelf1 * divs))
    return ( div * div_sz
             + (bracket * fold)
           )
end

function S_absolute( samp, divs, fold )
    local bracket = math.floor(samp / fold)
    local shelf   = (samp % fold) / fold
    local plateau = 0
    if type(divs) == 'table' then
        if divs[1] == 'ji' then
            --TODO needs log shaping i believe
            plateau = divs[2][math.floor(1.5+(shelf * #divs[2]))] - 1
        else
            plateau = divs[2][math.floor(1.5+(shelf * #divs))] / divs[1] 
        end
    else
        plateau = fold / divs * math.floor(0.5 + (shelf * divs))
    end

    --local plateau = type(divs) == 'table'
    --    and divs[math.floor(0.5 + (shelf * #divs))]
    --    or fold / divs * math.floor(0.5 + (shelf * divs))
    return plateau + bracket * fold
end

lydian = {12,{0,2,4,6,7,9,11}}
pentatonic = {12,{0,2,4,7,9}}
just_lydian = {'ji',{ 1/1, 9/8, 4/3, 11/8, 3/2, 10/7, 15/8}}
S_absolute( 0.1
          , lydian
          , 1
          )
