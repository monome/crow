# crow

An embedded lua interpreting usb<->cv bridge (sometimes for norns).
A collaboration by whimsical raps & monome

For a developer focused intro, see [readme-development.md](readme-development.md).

## what's a crow?

crow is many things, but here's some starters:

- Eurorack module. 2hp. +56mA / -16mA
- Hardware i/o: 2inputs, 4outputs, 16bit, [-5v,+10v] range
- Full lua environment, 8kB of local script storage
- USB device, for communicating text(!)
- i2c leader & follower, multiple crows can share responsibilities

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

[Druid](https://github.com/monome/druid) is a small utility for communicating with crow, both for realtime interaction and the uploading of full scripts.

A text editor alongside Druid provides an interactive platform for designing new patterns in a modular synth.

#### norns

crow integrates seamlessly with norns as a CV and II interface.

See the full [crow studies](https://monome.org/docs/crow/norns/) for a complete guide.

#### Max

A [crow] Max object provides a thin layer over a [serial] object which communicates to crow. The [crow] object accepts specific messages to query inputs,
set functionality, and drive outputs. Additionally chunks of lua code can be sent
directly to the object to control crow in a totally open manner.

Visit the [crow-max-and-m4l repo](https://github.com/monome/crow-max-and-m4l) to download and learn more.

#### Ableton

Leveraging the above max object, a set of Max4Live devices enable simple-yet-powerful interfacing between Ableton and a modular synth.

They empower Ableton as the center of a hybrid modular system:

- Create clocks, and clock-synced LFO's
- Automate CV outputs with variable smoothing
- Read CV inputs as MIDI
- Use CV inputs to remote-control Ableton

Visit the [crow-max-and-m4l repo](https://github.com/monome/crow-max-and-m4l) to download and learn more.

### Standalone

*Standalone* mode is intended to let crow perform functions without needing to connect to a host device. To support this, the user can upload scripts to crow which will run whenever the system is powered up.

The defining difference of using crow standalone is the manner in which events are handled. While in *satellite* mode, events send messages over USB. When in *standalone* the user must handle these events in their script.

In order to get your standalone program onto crow you'll need a text editor and a mechanism to talk to the serial port. *druid* above will be the typical choice for those familiar with the command line.

## Standalone: Writing crow scripts

A typical crow script is broken up into two main sections:

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
