# crow

An embedded lua interpreting usb<->cv bridge (sometimes for norns).
A collaboration by whimsical raps & monome

For a developer focused intro, see [readme-development.md](readme-development.md).

## what's a crow?

crow is many things, but here's some starters:
- Eurorack module. 2hp. +60mA, -15mA (TODO confirm).
- Hardware i/o: 2inputs, 4outputs, 16bit, [-5v,+10v] range
- Full lua environment, 64kB of local script storage
- USB device, for communicating text(!)
- i2c leader & follower, multiple crows can share responsibilities
- MIDI input on TRS cable (top-most input only)

With such a wide range of features, crow may become many different things to many
different people. The focus has been to create an infrastructure where minimal
customization is required to achieve many standard functions.

In general there are two classes of use-case, crow as *satellite*, and crow
*standalone*. Separating crow's functions along this boundary is useful for
some descriptive purposes, but of course your use-case may cross the division.
It's quite possible that a *satellite* use case may want to have crow running a
local script to enrich the remote features.

### satellites

crow was originally conceptualized as a CV-expander for *norns*, enabling close
integration with modular systems, or other devices with voltage control. The
*satellite* idea is a generalization of this proposition, where crow is subsidiary
to a host device.

These modes work by sending and receiving chunks of lua code over the USB connection.
crow will execute a received chunk immediately, enabling the host environment to
direct crow to act, or query crow's state such as the level at an input jack.

#### druid

TODO

This is the standalone computer REPL. Like a command-line hosted maiden. It is
designed to be a simple interface for live-coding & configuring crow.

General use-case would be a split terminal with text-editor on one side, and
druid on the other. druid provides direct access to crow's REPL so you can test out
code directly, then copy & paste into your script. Some hotkeys for auto-uploading
a file to crow would be useful.

crow has a lot of built-in documentation which can be queried with the 'help()'
method on most of the libraries. This will cause crow to send a chunk of lua code
which will be printed in the REPL window. These code chunks are specifically made
to enable a user to copy/paste them directly into their script.

#### norns

TODO

norns will add a set of functions for communicating with crow.

norns will create a set of event handlers that the user can redefine.

#### Max

A max object called [crow] is included in this project, though in future will be
included in the Max distribution. It is a thin layer over a [serial] object which
communicates to crow. The [crow] object accepts specific messages to query inputs,
set functionality, and drive outputs. Additionally chunks of lua code can be sent
directly to the object to control crow in a totally open manner.

The object then returns the default events (inputs, trigger events, timers etc)
in a form easily integrated into a Max patch. A simple usage would send new values
to the output jacks, and turn on 'change' mode for the inputs such that bangs are
received when the inputs pass a threshold.

#### Ableton

Leveraging the above max object, a set of Max4Live devices will soon be created
enabling some simple yet powerful interfacing between Ableton and a modular synth.

Initially these will be focused around using Ableton as the center of a system:
- Creating clocks, and clock-synced ramps
- Automatable CV outputs with variable smoothing
- Recording CV inputs as MIDI
- Using CV inputs to remote-control Ableton

In future this could be extended to allow the modular to drive Ableton directly
via clocks, but perhaps some other parameters too. We're hoping some more active
Ableton users will help extend these ideas, if not the implementation.

### Standalone

*Standalone* mode is intended to let crow perform functions without needing to
connect to a host device. To support this, the user can upload scripts to crow
which will run whenever the system is powered up.

The defining difference of using crow standalone is the manner in which events are
handled. While in *satellite* mode, events send messages over USB, when *standalone*
the user must handle these events in their script.

In order to get your standalone program onto crow you'll need a text editor and a
mechanism to talk to the serial port. *druid* above will be the typical choice for
those familiar with the command line. The Max object can be used too if you are
comfortable in that environment. Eventually maiden may support uploading directly
to crow(?).

**would love to hear thoughts on what kind of tool is best suited for this**

## Standalone: Writing crow scripts

A typical crow script is broken up into two main sections:

TODO

extend this...

### 1: The init() function
Here the script initializes parameters & you can describe the general functionality
of your script this will likely include declaring events that will be called at
runtime.

### 2: Event handlers
These functions handle events created by:
- timers
- input jacks
- i2c
- midi input

## crow commands: Controlling the crow environment

The below commands should be integrated into a host environment as macros or
commands. In particular, the user shouldn't need to worry about typing them
explicitly. norns should provide higher-level functions that send these low-level
commands & split up larger code pieces automatically.

The following commands are parsed directly from usb, so should work even if the lua
environment has crashed. nb: start/end script won't work correctly if the env is
down though. Use clearscript first.

nb: only the first char after the `^^` symbol matters, so the host can
simply send a 3 char command (eg `^^s`) for brevity & speed

- `^^startscript`: sets crow to reception mode. following code will be saved to a buffer
- `^^endscript`: buffered code will be error-checked, eval'd, and written to flash. crow returns to repl mode.
- `^^clearscript`: clears onboard flash without touching lua. use this if your script is crashing crow.
- `^^printscript`: requests crow to print the current user script saved in flash over usb to the host. prints a warning if no user script exists.
- `^^bootloader`: jump directly to the bootloader.
- `^^reset` / `^^restart`: reboots crow (not just lua env). nb: causes usb connection to be reset.
- `^^identity`: returns serial number.
- `^^version`: returns current firmware version.



### recovering from an unresponsive state

It's entirely possible to upload crow scripts that will make crow unresponsive
and require clearing of the on-board script.

#### Requesting user-script-clear

The gentlest way to deal with this situation is to send the `^^clearscript`
command over usb. You <will be able to> do this by typing `;c` or `^^c` in the
crow application<, or pressing a 'clear script' button in maiden?>, or pressing
the (^^clearscript) message box in max.

#### Using the bootloader

In extreme cases, your script might even make the low-level system become
unresponsive which will stop crow from responding to your `^^clearscript`
command.

See *Bootloader* at the bottom of this document.

## Input Library

crow's cv inputs can be used in three different ways: 'none', 'stream' and 'change'.
This setting is chosen independently for each of the two inputs, allowing each jack
to take on different roles.

- 'none' is the default, requiring you to manually query input values.
- 'stream' returns values at regular time intervals.
- 'change' sends an event when the input crosses a predefined threshold.

This choice of functionality is referred to as the input's 'mode'. to set input 1 to
'none' mode you can use `input[1].mode('none')`.

Or set the second input to 'stream' mode where values will be sent 10 times a second
(aka every 0.1 seconds) `input[2].mode('stream', 0.1)`.

In this way the `mode` function takes a variable number of arguments, depending on
which mode you've selected. If you don't supply all the arguments, the last value
you used will be applied, falling back to the default if it's your first time.

Remembering the number and order of these arguments can be a pain, so you can also
setup the mode in a different manner, explicitly naming your parameters:
```
input[2]{ mode = 'stream'
        , time = 0.1
        }
```
The above code will act identically to our above `mode` function call, but now it's
clear to us (and *future* us) what the `0.1` refers to- *time*!

This style is more useful for 'change' mode as there are more arguments:
```
input[1]{ mode       = 'change'
        , threshold  = 1.0
        , hysteresis = 0.2
        , direction  = 'rising'
        }
```
`input[1]` chooses the first channel (upper most jack on crow).
`mode = 'change'` means we'll receive events when the threshold is crossed.
`threshold = 1.0` sets the detection level to +1 volt. above this is 'high', and below is 'low'
`hysteresis = 0.2` modifies the 'threshold' value depending on which direction the
signal is moving. If the signal is low, output won't switch high until reaching 1.1V,
but once the signal has gone high it won't switch low until passing beneath 0.9V.
The difference here is 0.2V and is known as hysteresis. Hysteresis is useful to avoid
'bouncy' switching when the input signal is near the threshold point.
`direction = 'rising'` means the event will *only* happen when the threshold goes
from low to high. When the signal returns to the low state, no event is created.
Other options are 'falling' for only at the *end* of a gate, or 'both' which creates
events on both the high and low going transitions.

Additional modes are forthcoming in future updates for more sophisticated event
detection.

### Defaults for crow satellite

Set up input 1 to detect rising signals with a small amount of hysteresis
```
input[1]{ mode       = 'change'
        , threshold  = 1.0
        , hysteresis = 0.1
        , direction  = 'rising'
        }
```
Each time the signal passes above ~1.0V an event will be created on the host:

On norns:
```
function crow.change( channel, state )
    -- TODO. here's where you do the action!
    -- nb: 'state' will always be '1' in 'rising' mode
    --      but will tell you high/low as 1/0 in 'both'
    --      or remain as all zeroes in 'falling'
end
```

In Max the [crow] object will emit a message from the left output (change a b) where
'a' is the channel and 'b' the state.

Setting up a stream is very similar.
```
input[2]{ mode = 'stream'
        , time = 1.0
        }
```
resulting in the following remote event:
```
function stream( channel, value )
    -- TODO. do something with the stream of input values!
end
```
Or for Max, the message is similar: (input a b) where 'b' is the value.

The inputs can also be queried directly, rather than setting up a stream on a timer.
`input[1].query()` will send a response in the same manner as the stream above.

To do the same from Max, you can send the (get_cv x) message to the left input.


There is also a standard function call to setup these functions which is more terse:
```
input[1].mode( 'none' ) -- deactivated, but can still be queried with `.query()`
input[2].mode( 'stream', 0.05 ) -- sends 20 values per second
input[1].mode( 'change' ) -- default gate detector
```

There is *also* an assignment style:
`input[1].mode = 'stream'`
though it is limited to changing mode (no params) and should probably be removed(?)

### Standalone Inputs

The only real change when using the inputs on crow itself is you'll need to define
your own events.

The default stream action is defined as:
`input[1].stream = function(value) _c.tell('stream',1,value) end`

You can however redefine this to suit your own needs:
```
input[1].stream = function(value)
    -- here we echo the input to output channel 1
    -- we could set out[1].rate to smooth out changes from step to step
    output[1].level = value
end
```

A great use case for this is to allow crow's inputs to become triggers for i2c
enabled modules. The following snippet turns crow's first input into a momentary
gate which controls whether a remote W/ module is recording.

```
function init()
    input[1].mode( 'change' ) -- default gate detection
end

input[1].change = function(state)
    ii.wslash.record(state)
end
```

## Midi Library

crow's first Input doubles as MIDI input with the somewhat new TRS cable standard.

*nb: The pinout follows the official standard and so is compatible with Korg & Akai*
*(amongst others), but you'll need an adaptor for Novation & Arturia gear.*

Think of 'midi' as a fourth option for the Input library, albeit with very different
behaviour. Activating MIDI happens with any of the three input mode setters:
`input[1].mode = 'midi'`
or
`input[1].mode('midi')`
or
`input[1]{ mode = 'midi' }`

### Defaults for crow satellite
By default, MIDI mode simply forwards the messages to the host as raw MIDI data.
Currently all standard messages are supported except for SysEx commands.

TODO
On norns, one can treat the `crow.midi` event identically to the standard norns MIDI
events, using the `midi.to_msg` function to parse the input.

In Max the [crow] object will emit a message from the left output (midi data ...)
where `data` may be 1 to 4 numbers long. If you want to use the [midiparse] object
try: `[route midi] -> [zl iter 1] -> [midiparse]`. This will make crow feel like a
regular Max MIDI input.

### Standalone MIDI
Sure crow is great at being a midi / cv / i2c bridge to a host, but MIDI is far more
powerful when crow is running standalone. In this way your MIDI device can become
the user-interface for crow. CCs could control variables of a lua sequencer, or
notes could play Just Friends polyphonically...

The syntax for handling this behaviour is identical to norns, minus the connection
and 'ports' handling. crow only has one port! Try the below example to get started:
```
function init()
  input[1].mode = 'midi'
end

input[1].midi = function(data)
  local m = Midi.to_msg(data)
  if m.type == 'note_on' then
    print('on ' .. m.note)
  elseif m.type == 'note_off' then
    print('off ' .. m.note)
  elseif m.type == 'cc' then
    print('cc ' .. m.cc .. ' ' .. m.val)
  end
end
```

## Output Library & ASL

Each of crow's 4 outputs use an instance of the Output library to give control over
the CV values on each channel. The most basic actions are designed to be easy but
the library is built to be highly extensible.

Set output 1 to a 2 volts:
`output[1].volts = 2.0`

When assigning values via `volts` you can specify a `slew` time over which the output
will move toward the destination `volts`. For those familiar with monome's *teletype*
this should be a familiar model.
```
output[1].slew = 0.1
output[1].volts = 5.0
```
Above we set slew time to 0.1 seconds (100ms), and then start moving the output to
5 volts. The output channel will transition from the current location to the new
`volts` value over the time set by `slew`.

To return to instant actions, use `output[1].slew = 0`

### ASL

See `ref/asl-spec.lua` for some discussion of ASL.

ASL extends Lua with a syntax for describing musical 'actions'. These are dynamic
sequences of slopes, with a few special key words to connect them in musical ways.
The benefit is you can describe all kinds of envelopes, LFOs & sequences (with
portamento) directly, or write complex actions that use live inputs, or programmatic
values.

ASL is brand new and sharing your examples will help it mature!

### Output examples

ASL is used under the hood of the output library to implement the above mentioned
`volts` and `slew`. By using the power of ASL it's possible to chain together sets
of `volts` and `slew` settings, running them in sequence as they complete.

Here we set output 1 to the `action` of a 1 Hz low-frequency oscillator:
`output[1].action = lfo( 1.0 )`

You might notice that *nothing happened*. That's because `action`s need to be started
before they will run. Start the lfo with:
`output[1]()`

This separation of assignment & execution is useful in some cases, but sometimes
you just want the LFO *now*. You can do that by calling the output table with an
ASL like so:
`output[1]( lfo(1.0) )`

There's a list of different default actions like `lfo()`, `adsr()` in the 'Asl lib'
located in `lua/asllib.lua`.

You can of course define your own ASL generator functions, or use it directly. Below
we create a sawtooth LFO jumping instantly to 5 volts, then falling to 0 volts in
one second, then repeating indefinitely.
```
output[1].action =
    loop{ to( 5.0, 0.0 )
        , to( 0.0, 1.0 )
        }
```
Remember to start it by calling the output itself `output[1]()`.

Think of the `to()` function as a pair of `volts` and `slew` times. ASL just allows
you to chain these together in time, with some nice helpers like `loop`.

### directives

A number of special commands can be used to control the execution of an active ASL.
Restart the active ASL:
`output[1]('restart')`

Accepted directives are:
* 'restart' or 'attack' go to beginning of the ASL and starts. will enter `held{}`
* '' or 'start' steps forward to the next stage. will enter `held{}`
* 'release' exit an active `held{}` construct and do the first action
* 'step' steps forward to the next stage. does not affect `held{}` entry
* 'unlock' unsets the `lock` variable and steps forward.

Additionally a boolean (true/false) value can be used for traditional attack-release
control. This is designed to work well with the input libraries `change` mode. Here
input[1] will begin an ADSR when it goes high, and enter 'release' phase on low:

```
input[1].change = function(state)
    output[1](state)
end
output[1].action = adsr()
input[1].mode( 'change' )

```

### volts

It can be useful to query the current value of an output. Rather than setting 'volts'
as above, you can query it like so:
`v = output[1].volts`

Here we query the 'volts' of output 1 to set the rate for an lfo on output 2:
```
output[2].action =
    lfo( function() return output[1].volts end
       , 'linear'
       , 5.0
       )
```

## Metro library

crow has 7 user-assignable metronomes (or `metro`s) that can be used wherever you
need a sense of time. The metro library is borrowed from *norns* so if you're
familiar with that system this should be an easy transition.

### Indexed Metros

The fastest way to get a metro running, is to use the indexed approach. Try:
```
metro[1].event = function() print'tick' end
metro[1].time  = 1.0
metro[1]:start()
```
Which will set the first metro to execute every second, where it will print the word
'tick' to the host.

This can be really useful if you want to have a set of related timers like so:
```
for i=1,7 do
    metro[i].event = function() print(i) end
    metro[i].time = 1.0/i
    metro[i]:start()
end
```
Which will assign all seven timers to print their own index at the first seven
integer multiples of 1 second. That's a lot of messages printing! You can turn them
off with:
```
for i=1,7 do
    metro[i]:stop()
end
```

### Named Metros

The above indexed metros are great for setting up quick and basic timing patterns,
but sometimes you'll want to be more descriptive & explicit with the uses. For this
you can use the `Metro.init` library function to create a named metronome:

```
mycounter = Metro.init{ event = count_event
                      , time  = 2.0
                      , count = -1
                      }
```
The additional `count` argument allows to create timers that only repeats a limited
number of times before deactivating. By default `count` is set to `-1` which is
interpretted as 'repeat forever'.

As before, start the timer with the `start` method call, though note we call it on
our named metro:
`mycounter:start()`

This will begin calling your 'event', in this case `count_event` which has access to
the number of iterations that have occured since starting. You'll want to set it up
like this:
```
function count_event( count )
    -- TODO your function here!
end
```

You can then change parameters on the fly:
`mycounter.time = 10.0`
`mycounter.count = 33`
`mycounter.event = some_other_action`

And start/stop as you need. Note that each time you call `start`, the count is reset:
`mycounter:start()`
`mycounter:stop()`

## i2c support

crow supports acting as an i2c leader or follower. This allows it to control
other devices on a connected network, query those devices state, or itself
be controlled or queried by other devices. These many possibilities suggest
a wide range of varied use cases for you to discover!

### Pullups

i2c requires pullups on the data lines. crow can pull up the lines if needed, but
this function is off by default as some modules (such as Teletype, or powered bus
boards) may already be adding pullups.

If you're directly connecting crow to a single un-pulled-up module (such as Just
Friends) enable pullups with the following command:

```
(standalone) ii.pullup(true)
(norns) crow.ii.pullup(true)
```

Disable them by passing `false` instead.

### Leading the i2c bus

You can get a list of supported i2c devices by typing:
`ii.help()`

All the returned devices can themselves be queried for their available functions.
`ii.<module>.help()`, or eg: `ii.jf.help()`

These functions are formatted in such a way that you can directly copy-and-paste
these help files into your script.

#### Commands / Setters
Remote devices can be controlled with `commands`. These are listed first by the
`help()` functions. eg: `ii.jf.trigger( channel, state )`. These are typically called
'setters' when described in the teletype context.

You can call these like regular functions and they will send their commands over the
i2c bus to their destination.

#### Queries / Getters
Queries are values that can be requested from a connected device. This could be
asking a clock device for it's *tempo*, or an analog input device for the *voltage*
at one of it's inputs.

crow uses a query -> event model, which is different from teletype which has a
functional approach. In teletype, you call the getter, it requests the value,
waits to receive it, then returns that value as it's result directly.

crow's query -> event model separates the *request* from the *response*. While
this approach is a little more complex, it allows crow to do a great many *other*
things while it waits for a response to it's request.

First you use `ii.<module>.get( name, ... )`, which again can be copied directly
from the device's help() call. The `...` here refers to a variable list of arguments
which might be none at all! eg:
`ii.jf.get( 'run_mode' )`

Then you can declare an `event` action to handle the response from the device.
Copy it from the help() printout! it should look something like:
```
ii.jf.event( event, data )
    if event == 'run_mode' then
        -- the state of 'run_mode' is in the 'data' variable!
    end
end

```

Note that by default the `event()` defaults to a satellite action which is
`^^ii.<module>(<event>,<value>)`
for eg:
`^^ii.txi('value', 0.1)`

## Calibration

crow has an in-built calibration mechansim to allow the inputs and outputs to
accurately follow and output values. This functionality is primarily for use with
volt-per-octave signals, though of course this accuracy can be used for any number
of other purposes!

All modules come pre-calibrated from the factory, so you'll likely never need to
think about this, but just in case, recalibration and data inspection is allowed.

### Cal.test()

The `Cal.test()` function causes crow to re-run the calibration process.

**You must remove all patch cables from the jacks for this process to work correctly!**
Calling `test` without any arguments runs the calibration as per normal, while
runing `test('default')` will not run the calibration process, but instead return
to default values, in case you're having problems with the calibration process.

### Cal.print()

This helper function is just for debugging purposes. Calling it will simply dump
a list of values showing the scaling & translation of voltages that were measured
during the calibratin process. If you're really curious try resetting to the defaults
then printing, followed by a `test()` and `print` to see the difference.


## Bootloader

Flashing the crow requires setting up `dfu-util` on your laptop, downloading the new firmware, and getting crow connected in bootloader mode.

### Setup dfu-util

- Linux: `apt-get install dfu-util` (or similar)
- MacOS: install [homebrew](https://brew.sh) and then run `brew install dfu-util` from the command line.
- Windows: get a [win32 binary](http://dfu-util.sourceforge.net)

### Get new firmware

- https://github.com/monome/crow/releases


### Activate bootloader

With the crow connected to druid (or a similar utility) you can enter the bootloader with a `^^b` message, which will instantly reset the module and take you to the bootloader.


### Forcing the bootloader

In case both of the above don't work, you can manually force the bootloader to
run by placing a jumper on the i2c header and restarting (power-cycling) crow.

The jumper should bridge between either of the centre pins to either of the
ground pins (ie the pins closest to the power connector, indicated by the
white stripe on the pcb). In a pinch you can hold a (!disconnected!) patch cable
to bridge the pins while powering on the case.

![](doc/img/crow-dfu.jpg)

### Flashing the update

Execute the `flash.sh` command which is included in the release .zip file. The actual firmware file that is uploaded is`crow.bin`.

For example if your file was extracted to `~/Downloads/crow-1.1.0` type this on the command line:

```
cd ~/Downloads/crow-1.1.0
./flash.sh
```

You'll see something like:

```
dfu-util 0.9

Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
Copyright 2010-2016 Tormod Volden and Stefan Schmidt
This program is Free Software and has ABSOLUTELY NO WARRANTY
Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

dfu-util: Invalid DFU suffix signature
dfu-util: A valid DFU suffix will be required in a future dfu-util release!!!
Deducing device DFU version from functional descriptor length
Opening DFU capable USB device...
ID 0483:df11
Run-time device DFU version 011a
Claiming USB DFU Interface...
Setting Alternate Setting #0 ...
Determining device status: state = dfuIDLE, status = 0
dfuIDLE, continuing
DFU mode device DFU version 011a
Device returned transfer size 1024
DfuSe interface name: "Internal Flash   "
Downloading to address = 0x08020000, size = 290876
Download	[=========================] 100%       290876 bytes
Download done.
File downloaded successfully
```

### Troubleshooting

If you get an error: `dfu-util: No DFU capable USB device available` this means the bootloader is not running and connected to the laptop.

You can type `dfu-util -l` to list the connected bootloader devices.
