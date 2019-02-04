# crow
an embedded lua interpreting usb<->cv bridge (for norns).
a collaboration by whimsical raps & monome


## toolchain setup

### prereq

- arm-none-eabi-gcc 4.9.3: https://launchpad.net/gcc-arm-embedded/4.9
- fennel: `luarocks install fennel` *nb: requires lua 5.3 for now*

### get the project
- `git clone --recursive https://github.com/trentgill/crow.git`
- `cd crow`
- `git submodule --init` *nb: will take a while to download*

### flashing
you can either use an stlink / discovery board to flash the firmware using the edge connector.
you'll need an st-link & an edge-connector adaptor <oshpark link>
if you want to change the bootloader, you'll need to use this setup
- stlink: https://github.com/texane/stlink#installation

or you can flash the module over the panel-mount usb jack using dfu-util
- dfu-util: apt-get install dfu-util

### building
- `make` / `make all` build the project binary
- `make clean` remove all binary objects to force a rebuild on next `make`

*for st-link disco board*
- `make flash` build & flash the main program
- `make boot` build & flash the bootloader
- `make erase` only useful to give the module an original position

*for dfu programming over usb*
- `make dfu` build & flash the main program

## writing crow scripts
a typical crow script is broken up into two main sections:

### 1: the init() function
here the script initializes parameters & you can describe the general functionality of your script
this will likely include declaring events that will be called at runtime

### 2: event functions
these functions handle events created by:
- timers
- input jacks
- i2c
- midi input

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


## recovering from an unresponsive state
it's entirely possible to upload crow scripts that will make crow unresponsive
and require clearing of the on-board script.

### requesting user-script-clear
the gentlest way to deal with this situation is to send the `^^clearscript`
command over usb. you <will be able to> do this by typing `;c` or `^^c` in the
crow application<, or pressing a 'clear script' button in maiden?>, or pressing
the (^^clearscript) message box in max.

### using the bootloader
in extreme cases, your script might even make the low-level system become
unresponsive which will stop crow from responding to your `^^clearscript`
command.

in this case you can enter the bootloader with a `^^b` message, which will
instantly reset the module and take you to the bootloader.

here you can use `dfu-util` to clear the user-script flash zone.

### forcing the bootloader
in case both of the above don't work, you can manually force the bootloader to
run by placing a jumper on the i2c header, and restarting crow.

the jumper should bridge between either of the centre pins to either of the
ground pins (ie the pins closest to the power connector, indicated by the
white stripe on the pcb). in a pinch you can hold a (!disconnected!) patch cable
to bridge the pins while powering on the case.

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




## metro lib

### norns style
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


### 'assign_all'
sometimes you just need a bunch of timers without wanting to name each timer and set
explicit actions. in this case there's a shorthand to get all the metros setup and
running. just add:
    `metro = Metro.assign_all()`
to your `init()` function.

this makes `metro` a table of metros with default events assigned. to start them
running, use the 'start' method call with a time.

```
--- Assign all the metros, and set up 3 phasing timers at 1,3 and 5 second intervals
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








## input script examples

### default actions for crow-as-remote

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


### using inputs inside a crow script

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


## i2c support

crow supports acting as an i2c leader or follower. this allows it to control
other devices on a connected network, query those devices state, or itself
be controlled or queried by other devices. these many possibilities suggest
a wide range of varied use cases for you to discover!

### using i2c
you can get a list of supported i2c devices by typing:
`ii.help()`

all the returned devices can themselves be queried for their available functions.
`ii.<module>.help()`, or eg: `ii.jf.help()`

these functions are formatted in such a way that you can directly copy-and-paste
these help files into your script.

#### commands / setters
remote devices can be controlled with `commands`. these are listed first by the
help() functions. eg: `ii.jf.trigger( channel, state )`. these are typically called
'setters' when described in the teletype context.

you can call these like regular functions and they will send their commands over the
i2c bus to their destination.

#### queries / getters
queries are values that can be requested from a connected device. this could be
asking a clock device for it's *tempo*, or an analog input device for the *voltage*
at one of it's inputs.

crow uses a query -> event model, which is different from teletype which has a
functional approach. in teletype, you call the getter, it requests the value,
waits to receive it, then returns that value as it's result directly.

crow's query -> event model separates the *request* from the *response*. while
this approach is a little more complex, it allows crow to do a great many *other*
things while it waits for a response to it's request.

first you use `ii.<module>.get( name, ... )`, which again can be copied directly
from the device's help() call. the `...` here refers to a variable list of arguments
which might be none at all! eg:
`ii.jf.get( 'run_mode' )`

then you can declare an `event` action to handle the response from the device.
copy it from the help() printout! it should look something like:
```
ii.jf.event( event, data )
    if event == 'run_mode' then
        -- the state of 'run_mode' is in the 'data' variable!
    end
end

```

### adding i2c support for a new device
in order to encourage wide-ranging support for all i2c capable devices, crow
requires only a single file to be added per device. this lua file is a simple
declarative specification of how that device communicates on the i2c bus.

these files live in `crow/lua/ii/<device_name>.lua`

#### beyond the obvious
take a look at `jf.lua` as an example which adds support for mannequins'
'just friends' module. this module takes advantage of most of the existing
features of the build system.

the first 4 lines are global settings stating this module is called 'just friends',
made by 'mannequins' and can be talked to at the hexadecimal address 0x70. the
`lua_name` field *must* match the filename (jf.lua -> 'jf'), and is the name
by which users will refer to this device in their scripts eg: `ii.jf.trigger()`.

following this header is a big table called 'commands' which is itself full of
tables, one for each 'setter' command the device can receive. a 'setter' in this
context typically allows crow to remotely-control a parameter or event. more
generally a 'setter' is any command that doesn't return any values.

- 'name' is how the user will refer to this command: eg: name=trigger -> ii.jf.trigger()
- 'cmd' is the number the remote device expects to trigger the 'name'd function. see https://github.com/monome/libavr32/blob/master/src/ii.h for a starting point.
- 'docs' is an optional string describing the functionality of the command.
- 'args' is a table of tables, each inner-table describing a command argument.

##### no arguments
in the simplest case, the command doesn't require *any* arguments, eg: ii.jf.reset()
in which case you can omit the 'args' descriptor altogether.

##### 1 argument
if the command expects a single argument you can declare it as a table directly.
eg: args = `{ 'name', type }`
it is also allowed to use the nested table syntax: `args = { { 'name', type } }`

##### 2+ arguments
the most general case allows for any number of arguments, each defined as a table
within the args table like so:
```
args = { { 'arg1', type }
       , { 'arg2', type }
       }
```

##### what's a type?!
each argument has both a *name* and *type*. the *name* is purely for documentation, so
the user knows at a glance what that argument means. the *type* refers to the
low-level representation of the value.

available types are:
- void  -- the lack of an argument (typically not needed)
- u8    -- unsigned 8-bit integer (0,255)
- s8    -- signed 8-bit integer (-128,127)
- u16   -- unsigned 16-bit integer (0,65535)
- s16   -- signed 16-bit integer (-32768, 32767) {teletype's default}
- float -- 32bit floating point number

*u16* and *s16* expect MSB before LSB.
*float* is little-endian

refer to the documentation or source-code for the device you're supporting to
determine what types are used for these types.

##### get
many devices built for use within the teletype ecosystem have a matching 'getter' to
go with each 'setter'. so while `JF.RMODE 1` sets 'run_mode' to 1, `JF.RMODE` without
any argument, will *query* the value of 'run_mode' returning that as a result.

under the hood, these matching getters & setters are typically given the same 'cmd'
value, with the getter offset by 0x80. the *last* argument in the list is omitted
when calling the command, and instead is expected as the return value.

by setting `get = true`, a getter will be auto-generated with the above assumptions.

if you have getters that *don't* use these conventions, you can articulate them
explicitly. >>>

##### getters
a single function is provided for getting, or *querying*, values from a connected
device. this is: `ii.jf.get( name, ... )`. to define 'gettable' values, you add them
to the 'getters' table in much the same way as the 'commands' table.

- `name` is the name by which to query the state
- `cmd` is the number corresponding the above name (see your devices docs)
- `docs` is an optional string describing what will be returned
- `args` is identical to the 'commands' version. can be omitted if no arguments.
- 'retval' is like a single `arg`. it should be a table with a string then a type.


#### some gotchas
to simplify the build process, the 'lua_name' variable currently must match
the name of the .lua file (ie. `jf.lua` must have `lua_name = 'jf'`)

#### future development
while extensive, this descriptive framework could be extended to add any or all of
the below features. see the github issues list for discussion:

- remove requirement that filename match lua_name
- lua_name aliases to allow eg: 'jf' or 'justfriends' to access the same table
- additional argument & return_value types
    - fractional types with radix. eg: fract13 converts a 13bit int to 0-1.0 float
- multiple return values (eg: ii.jf.get('retune',1)) could return both `num` and `denom`


#### the below is really for a time when we have fract types
different values require different levels of precision depending on their use. the
i2c bus began with monome's teletype device, a system itself based on a *signed 16bit
integer* virtual machine. crow on the other hand, is based around a *32bit floating
point* virtual machine.









