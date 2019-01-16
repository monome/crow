# crow
an embedded lua interpreting usb<->cv bridge (for norns).
a collaboration by whimsical raps & monome


## toolchain setup

### prereq

- fennel: `luarocks install fennel`

## writing crow scripts
a typical crow script only has a few components:
- (almost?) always an `init()` function
- timer events
- input events
- i2c events
- midi events

## crow commands
the below commands should be integrated into a host environment as macros
or commands. in particular, the user shouldn't need to worry about typing
them explicitly. norns should provide higher-level functions that send these
low-level commands & split up larger code pieces automatically.

the following commands are parsed directly from usb, so should work
even if the lua environment has crashed. nb: start/end script won't
work correctly if the env is down though. use clearscript first.

nb: only the first char after the `^^` symbol matters, so the host can
simply send a 3 char command (eg `^^s`) for brevity & speed

- `^^startscript`: sets crow to reception mode. following code will be saved to a buffer
- `^^endscript`: buffered code will be error-checked, eval'd, and written to flash. crow returns to repl mode.
- `^^clearscript`: clears onboard flash without touching lua. use this if your script is crashing crow.
- `^^printscript`: requests crow to print the current user script saved in flash over usb to the host. prints a warning if no user script exists.
- `^^bootloader`: jump directly to the bootloader.
- `^^reset` / `^^restart`: reboots crow (not just lua env). nb: causes usb connection to be reset.

### communication details
if you're writing a host environment to control/upload to crow, there are some
limitations to keep in mind:

the usb connection will break up any messages into 64byte blocks. if you want
to send a codeblock longer than this, you need to use the multiline helper.
first send a command of 3 backticks "```", then your codeblock, then another
3 backticks. this should be hidden from the end-user.

however! if you are sending a full lua script and it's over 1kB, it must
be broken up into smaller chunks. if you are in script-transmission-mode then
you can arbitrarily break up the message every 1kB. if you're sending a giant
codeblock directly to the lua repl, you'll need to split the message so the
lua interpreter sees each portion as valid code, as each multi-line segment
is evaluated upon reception.



## using the [crow] max/pd object


### accessing cv inputs
crow's cv inputs can be used in three different ways when acting as a remote via
max/pd: 'none', 'stream' and 'change'. this setting is chosen independently for
each of the two inputs, allowing each jack to take on different roles.

    'none' is the default, requiring you to manually query input values.
    'stream' returns values at regular time intervals.
    'change' sends an event when the input crosses a predefined threshold.

this choice of functionality is referred to as the input's 'mode'. to set input 1 to
'none' mode you can use:
    `input[1].mode('none')`

or set the second input to 'stream' mode where values will be sent 10 times a second
(aka every 0.1 seconds) use:
    `input[2].mode('stream', 0.1)`

in this way the `mode` function takes a variable number of arguments, depending on
which mode you've selected. if you don't supply all the arguments, the last value
you used will be applied, falling back to the default if it's your first time.

remembering the number and order of these arguments can be a pain, so you can also
setup the mode in a different manner, explicitly naming your parameters:
```
input[2]{ mode    = 'stream'
        , time    = 0.1
        }
```
the above code will act identically to our above `mode` function call, but now it's
clear to us (and *future* us) what the `0.1` refers to, time!

this style is more useful for 'change' mode as there are more arguments:
```
input[1]{ mode       = 'change'
        , threshold  = 1.0
        , hysteresis = 0.2
        , direction  = 'rising'
        }
```
`input[1]` chooses the first channel (upper most jack on crow).
`mode = 'change'` means we'll receive events when the threshold is crossed.
`threshold = 1.0` sets the detection level to 1 volt. above this is 'high', and below is 'low'
`hysteresis = 0.2` modifies the 'threshold' value depending on which direction the
signal is moving. if the signal is low, output won't switch high until reaching 1.1V,
but once the signal has gone high it won't switch low until passing beneath 0.9V.
the difference here is 0.2V and is known as hysteresis. hysteresis is useful to avoid
'bouncy' switching when the input signal is near the threshold point.
`direction = 'rising'` means the event will *only* happen when the threshold goes
from low to high. when the signal returns to the low state, no event is created.
other options are 'falling' for only at the *end* or a gate, or 'both' which creates
events on both the high and low going transitions.

additional modes are forthcoming in future updates for more sophisticated event
detection.

>>> receiving the messages
each of the above modes interact differently with the [crow] object.

to use 'none' mode, you can send the message `(get_cv x)` where `x` is either 1 or 2
for the input channel you want to access. this will cause the first outlet to return
a message `(return_cv x v)` where `x` is the same channel, and `v` is the value.
*important*: unlike other objects, [crow] will not wait for the response from the
crow module, instead it will request the value, then allow max to carry on with other
processes. when the response is received, it will continue out of the [crow] object
left output. this can affect ordering of events in your patch!

'stream' mode reception is identical to 'none', creating `(return_cv x v)` messages
the difference being these messages are not individually requested, but automatically
received.

'change' mode instead creates a `(return_change x s)` message, where `x` is the
channel, and `s` is the state, `1` for high and `0` for low. note: if you set
direction to 'rising' you'll only ever receive the number `1`.

>>> advanced return values
while the above describes the standard usage, it's possible to redefine what happens
when a 'stream' or 'change' message is returned. under the hood, these are
implemented as simple functions in the input tables. the defaults being:
```
in[1].stream = function( value )
    get_cv(1)
end

```
and
```
in[1].change = function( state )
    _crow.tell( 'change', 1, state )
end
```
these event handlers can be redefined as you see fit




### metro lib

## norns style
each time you want a new timer you can assign it with some default params:
```
mycounter = Metro.init{ event = count_event
                      , time  = 2.0
                      , count = -1
                      }
```
then start it:
`mycounter:start()`
which will begin calling the your 'event', in this case count_event.
you'll want to set it up like this:
```
function count_event( count )
    -- TODO
end
```
you can change parameters on the fly:
`mycounter.time = 10.0`
`mycounter.count = 33`


## 'auto_metros'
sometimes you just need a bunch of timers without wanting to name each timer and set
explicit actions. in this case there's a shorthand to get all the metros setup and
running. just add:
    `metro = Metro.auto_metros()`
to your `init()` function.

this makes `metro` a table of metros with default events assigned. to start them
running, use the 'start' method call with a time.

```
--- Turn on auto_metros, and set up 3 phasing timers at 1,3 and 5 second intervals
function init()
    metro = Metro.assign_all()

    metro[1]:start( 1.0 )
    metro[2]:start( 3.0 )
    metro[3]:start( 5.0 )
end
```

by default, this will call a remote function:
```
function metro(channel, count)
    -- TODO
end
```

you can set custom times:
`metro[1].time = 0.1`

and stop a metro:
`metro[1]:stop()`

restart a stopped timer with a new time & count value:
```
metro[1].start{ time  = 1.2
              , count = 100
              }
```

or add aliases if you don't want to remember them all by number:
`my_hourly_reminder = metro[1]`

or redefine the event if you want to change functionality:
`metro[1].event = function(count) print(count) end`








### input script examples

## default actions for crow-as-remote

set up input 1 to detect rising signals with a small amount of hysteresis
```
function init()
    input[1]{ mode       = 'change'
            , threshold  = 1.0
            , hysteresis = 0.1
            , direction  = 'rising'
            }
end
```
each time the signal passes above ~1.0V this event will be called on the host:
```
function change( channel, state )
    -- TODO. here's where you do the action!
    -- nb: 'state' will always be '1' in 'rising' mode
    --      but will tell you high/low as 1/0 in 'both'
    --      or remain as all zeroes in 'falling'
end
```

setting up a stream is very similar ( and can be different between channels )
```
function init()
    input[2]{ mode = 'stream'
            , time = 1.0
            }
end
```
resulting in the following remote event
```
function ret_cv( channel, value )
    -- TODO. do something with the stream of input values!
end
```

there is also a standard function call to setup these functions which is more terse:
```
function init()
    input[1].mode( 'none' )
    input[2].mode( 'time', 0.05 )
end
```


## using inputs inside a crow script
fundamentally the only real change when using the inputs on crow itself is you'll
need to define your own events.

the default stream action is defined as:
`input[1].stream = function(value) _c.tell('ret_cv',1,value) end`

you can however redefine this to suit your own needs:
```
input[1].stream = function(value)
    -- here we echo the input to output channel 1
    -- we could set out[1].rate to smooth out changes from step to step
    out[1].level = value
end
```

a great use case for this is to allow crow's inputs to become triggers for i2c
enabled modules. the following snippet turns crow's first input into a momentary
gate which controls whether a remote W/ module is recording.

```
function init()
    input[1].mode( 'change' ) -- default gate detection
end

input[1].change = function(state)
    ii.wslash.record = state
end
```











