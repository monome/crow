#crow development setup

crow development can mean many things, but if you want to write anything beyond a user script, you'll need to setup the dev environment so you can build binaries. A binary can then be flash to crow using the DFU bootloader over USB. If you want to work on the bootloader itself, or want a slightly easier workflow you'll need an ST-Link and an edge-connector (see below).

## Toolchain setup

### Prerequisites

- [arm-none-eabi-gcc 4.9.3](https://launchpad.net/gcc-arm-embedded/4.9)
- lua 5.2 or higher
- luarocks
- fennel: `luarocks install fennel`
- dfu-util 0.8+

### Get the project
- `git clone --recursive https://github.com/trentgill/crow.git`
- `cd crow`
- `git submodule --init` *nb: will take a while to download*

### Building

- `make` build the project binary (** see nb below**)
- `make clean` remove all binary objects to force a rebuild on next `make`

*nb: currently the Makefile is a little broken and you'll need this*: (see issue #42)
- `make clean`
- `mkdir build`
- `lua util/ii_gen.lua`
- `make`

#### DFU Programmer

For dfu programming over usb, replace `make` above with:
- `make dfu`

This will build the project and then attempt to flash it over the crow USB port.

#### ST-link / Discovery board

Using an st-link, or stm discovery board as the programmer is also possible but you'll need:
- st-link, stm discovery board, or compatible programmer
- 'edge piece' [pcb](https://oshpark.com/shared_projects/lMkbP9a9)
- [edge connector](https://www.digikey.com/product-detail/en/7-5530843-7/A101966-ND/2310829)
- 6pin male [header](https://www.digikey.com/product-detail/en/sullins-connector-solutions/PREC006SFAN-RC/S1212EC-06-ND/2774968)

Or if you're adventurous, you can get by without the pcb and just wire a 2x3 header directly to the edge connector.

Pinout for SWD port on discovery board (and thus on the edge-piece 1x6pin connector):
1. vdd target (ie 3v3 from crow)
2. SWD clock
3. Ground
4. SWD data i/o
5. !Reset
6. SWO (ie 'TRACE' or debug pin)

The edge connector pin out is:
- top-side starting at rear-edge: TRACE, SWD clock, !RST
- bottom-side starting at rear-edge: 3v3, SWD data, Ground

Once you've got the hardware sorted, you can:
- `make boot` build & flash the bootloader
- `make flash` build & flash the main program
- `make erase` erase the whole program including calibration metadata & userscript

TRACE

I discovered the 'TRACE' pin functionality after we ordered the hardware, so
unfortunately it isn't hooked up on the crow hardware itself. You can cut a tiny
trace & solder a wire to the uC's TRACE pin, but it's not recommended. If you
really want that functionality I'm (@trentgill) happy to do the mod for you.

### Debugging
Presently debugging is limited to printf style, and you'll need to solder on the
6pin UART header on the crow module. The reset button is recommended too as it
invariably comes in handy.
- SMT [6pin header](https://www.digikey.com/product-detail/en/harwin-inc/M20-8890645/952-3235-ND/6565719) for UART
- reset [switch](https://www.digikey.com.au/product-detail/en/panasonic-electronic-components/EVQ-PUA02K/P10847SCT-ND/286348)

Any call to printf() will redirect to the UART port. The port communicates at
baud of 115200 and other standard settings in serial port connections. I typically
use `minicom` for observing the printout.

If you have suggestions for greater debug controls, I'd love to hear them!

### Build process
The makefile is pretty unwieldy, but here's the basic approach:

1. Compile all `.fnl` files to `.lua`, specifically for the `l2h.fnl` script
2. Run the `util/ii_gen.lua` script to autogenerat i2c links of anything in `lua/ii/*` -> results put into `build/*`
3. Wrap all remaining `.lua` files in `lua/*` and `build/*` into c-headers with the `l2h.lua` script
4. Build the C objects, including lua which is built directly into the binary
5. Run the linker and create the binary

Then you can upload to the device automatically with `make dfu` or `make flash`.

If you want to improve the makefile see issue #42.
If you want to improve the way lua code is included in the binary see issue #40.

## Project structure

- `ll/*`: all the low-level hardware drivers written in C using stm HAL.
- `usbd/*`: USB device driver for TTY communication to host.
- `lib/*`: hardware-agnostic C APIs to the LL drivers & system functions.
- `lua/*`: crow system providing lua APIs to the platform.
- `lua/ii/*`: descriptor files for supported i2c devices.

### Linking C functions and Lua

The file `crow/lib/lualink.c` contains the interface between the C infrastructure
and lua runtime. If you need to extend this interface, a wrapper function should
be created and linked in the `libCrow` struct.

Functions calling from Lua to C must follow the function prototype:
`static int _function_name( lua_State* L );`

Lead your C function with an underscore, and see the Lua docs for how to pass
arguments to and from these functions.

Callbacks to Lua are defined at the end of this file, by convention named
`L_handle_eventname` where 'eventname' is your callback's identifier.

Note: we'll need an event-handler in C at some point as presently all callbacks
are executed directly, calling into Lua which can lead to the platform going
unresponsive if the Lua callbacks have too much going on.

### Including Lua files in the binary

If you add a new `*.lua` file to the project at `lua/*.lua` you'll need to include
it in `lib/lualink.c` so as to have it built into the binary:

When these files are included in the project, their filenames are manipulated
automatically, and they're wrapped into long string arrays. Thus if you have a file
at `lua/example.lua` it will wrapped into `lua/example.lua.h` and will contain a
single `char*` array with the name `lua_example` containing the whole lua file
as raw text.

To include it in the binary add an `#include` at the top of the file with `.h`
appended to your filename.

To then allow `dofile()` or similar to find this code, you need to add a link to
the struct `Lua_libs` immediately after the includes.
    This should contain a string representing the path to the file, where `/` is
    replaced by `_` and the file extension is dropped. Followed by a the array
    identifier, identical to the string, but without quotes.

Nb: this does *not* apply to i2c descriptors.

### Flash layout

Crow has 512kB of Flash memory which is segmented as follows:
- 48kB, DFU bootloader (0x08000000)
- 16kB, calibration data (0x0800C000)
- 64kB, user-script (0x08010000)
- 384kB, crow application (0x08020000)

The DFU bootloader is protected and shouldn't be accessible from the main program.
Changing the bootloader requires a hardware programmer, and special PCB (see above).

The calibration data section contains automatic calibration information calculated
when the unit is first programmed. This should not need to be manipulated directly,
instead the calibration mechanism is in `ll/adda.c`. A lua hook should be provided
to allow the user to execute `io_recalibrate()`.

The 'user-script' location is where a lua program can be uploaded to. At present
this is simply a lua program as raw text, offset by a single 32bit word. The leading
word contains the length of the user-script in bytes, plus a status byte so the
system can query whether there is currently a program loaded. If no program is found
the system will load the script at `lua/default.lua`.

The crow application section includes all of the crow standard libraries in lua,
along with the C application.

Further details in `lib/flash.c`

### The REPL

Crow communicates with a host over a serial port. The C API is in `lib/caw.c` along
with some basic parsing of system level commands. These are discussed... elsewhere.

The parser is very basic, but just tries to capture complete lua chunks which are
queued for later processing. The results of the parsing are handled in the `main.c`
`while(1)` loop.

Direct commands are forwarded through the `REPL_eval()` function which chooses
between 2 functions depending on the REPL state:
1. Forwards the deatils to `Lua_eval()` to be executed immediately.
2. Forwards to `REPL_receive_script()` to save the incoming text to flash.

In the first case, the incoming serial stream is expected to be a well-formed lua
script and is called directly with `lua_pcall()`. If the script contains errors
they will be reported back over the USB port with whatever info is available.

Switching between modes is accomplished by sending a *crow command* alerting the
device that, until further notice, the following serial data should be saved for
later use as a full lua script. Once the full script is sent, the user ends with
another *crow command*. On completion of this sequence a number of things happen:
- serial stream is validated for correct lua syntax
- lua code is loaded with `lua_pcall()` again checking for errors
- stream is saved into flash memory, and the 'user-script-present' bit is set
- temporary buffer holding the script is destroyed
- REPL switches back to direct mode, where it will process incoming data directly

### Crow commands

Crow commands are special symbols that are intercepted before being passed through
to the lua environment. These commands control meta-functionality of the input
stream and allow a remote-host to control the hardware in special ways.

All crow commands start with the crow symbol: `^^`. They then choose the action
based on the first character following the symbol. Full names are listed here for
mnemonic benefit:
- `^^bootloader`: jump directly to the bootloader.
- `^^reset` / `^^restart`: reboots crow.
- `^^startscript`: sets crow to reception mode. following code will be saved
- `^^endscript`: saved code above will be written to flash. crow returns to repl mode
- `^^clearscript`: clears onboard user script. use if your script is crashing crow
- `^^printscript`: the current user script is sent over usb to the host

#### Bootloader

To upload a new crow firmware, you must first tell crow to enter bootloader mode.
This can be accomplished in Lua if you need your script to shutdown gracefully, or
can be called directly from the repl with the command `^^b`.

#### Reset

Crow can be restarted with the `^^r` command which has the same effect as turning
the power on and off on your system. This is useful if your script has crashed crow,
or if you are designing / debugging a certain start-up procedure.

#### Uploading a user-script

The first pair of commands works together, in the structure:
```
^^s
<lua chunk>
<another lua chunk>
^^e
```
Thus placing the two chunks into flash memory after being validated.

#### Clearing the user-script

As the user-script is loaded automatically whenever crow starts, if your script
causes the system to lockup or crash, you'll need to clear or overwrite the script.

To clear the existing script use `^^c` which will erase the current contents of
the user-script, and crow will load the default script on next load.

This can be very handy if you want to switch to the default behaviour part-way
through a performance.

#### Printing the user-script

On `^^p` request, crow will print the current user-script over the serial port to
the curious host. The code on crow is stored as plain lua text, so it should come
back exactly as you had entered it into the device.

This function can be used as a rudimentary 'save' capability, where the printed
script is saved into a text file.

A `druid` extension that adds some high-level script management functionality
that uses this would be a great addition.

#### Communication details

If you're writing a host environment to control/upload to crow, there are some
limitations to keep in mind:

The usb connection will break up any messages into 64byte blocks. If you want
to send a codeblock longer than this, you need to use the multiline helper.
first send a command of 3 backticks "```", then your codeblock, then another
3 backticks. This should be hidden from the end-user.

However! If you are sending a full lua script and it's over 1kB, it must
be broken up into smaller chunks. If you are in script-transmission-mode then
you can arbitrarily break up the message every 1kB. If you're sending a giant
codeblock directly to the lua repl, you'll need to split the message so the
lua interpreter sees each portion as valid code, as each multi-line segment
is evaluated upon reception.

### Signals

TODO

The signal i/o for the input and output jacks happens in the `lib/io.c` file in
`IO_BlockProcess()`. Signals are generated at 48kHz. This function processes a
block of samples per invocation. The number of samples is available in `b->size`
and is currently 32samples as of this writing.

All signals are processed as floating point numbers representing voltages directly.
Thus the range is [-5.0, 10.0] and anything beyond these values will be clipped to
the maximum.

The `IO_block_t` type is a struct described in `ll/adda.h` and should be used to
access the signals and metadata in the block processor.

#### Inputs

The input jacks are only sampled once per block, but the `b->in[channel][]` array
is linearly interpolated from block to block. Instantaneous input values are also
available directy from the low-level driver to avoid this smoothing, and for access
without having to change the block process with `IO_GetADC(channel)`. The linear
interpolation could be updated to something more useful?

The block process passes the input buffers to the Detect library implemented in
`lib/detect.c` which analysis the input for events that should generate events in
the lua environment. Event descriptors are sent from the lua Input library, which
define what will cause event interrupts to occur.

#### Input Dev Roadmap: More detection modes

The principle area of extension for the Input libraries is in the Detect library.
At present the library is only capable of a simple hysteretic threshold detection.
This is sufficient for using the inputs as trigger/gate detectors, but not much else.

Additional modes in the style of `change` will be added specifically covering:
- window: is the signal above / inside / below a defined window (could be a list)
- scale: create events when signal passes thresholds within a defined scale
- quantize: like 'scale' but with arbitrary divisions. (ie 'scale' is 12tone ET)
- ji: a just-intonation version of 'scale' where the scale is defined as ratios

The syntax of these are described in Issue #14.
A working example in Lua is available in `crow/ref/shaper.lua` at the end of file.

#### Outputs

Outputs are generated directly from the Slope library in `lib/slope.c` and again
the configuration of these slope generators is directly controlled from lua, via ASL
descriptors.

The output vector is generated by the `S_step_v()` function and will handle
breakpoints (ie. when the slope reaches it's destination) by calling into lua to
request the next slope description.

#### Output Dev Roadmap: Slope Shapes

Currently only Linear slopes are supported. A range of slopes should be available
to the user, and eventually some meta-functionality & even user-definable curves.

The current list are those from Just Friends, as they're in the wrDsp already:
- Linear
- Exponential
- Logarithmic
- Sine (really cosine)
- Square

The above are easly implemented with lookup tables, or very simple transfer
functions. Thus they are easily described with an enum, or even just strings in lua.
Some meta-shapers, or more accurately *shape-replacers* would be:

'Passthrough' would allow the Input jack to be passed to the output jack for the
duration of the slope segment. This requires the input array to be passed to each
Slope such that they all have access. This could be very useful along with the
composite slopes following (eg. adding offset to an input signal). The user should
be able to choose which input channel to follow.

'Generators' are a class of replacers that generate a signal to be inserted during a
slope segment. One simple example would be 'noise' where white(?) noise is inserted.

#### Output Dev Roadmap: Absolute Shapes

The above shapers are to be applied by treating the origin & destination as the
limits of the full slope sweep. Conversely *absolute shapes* are applied relative
to the actual values passing out of the slope generator.

The clear use-case of this functionality is for highly-flexible quantization. A
simple example is found at the end of `ref/shaper.c` called `S_absolute()`, whereby
a sample is quantized into it's nearest scale-division (here chromatic).

Additionally this functionality is useful if the user desires to shape the
distribution of output values to some specific range. An absolute-exponential would
pull the distribution toward zero, and would effectively zero-out negative values.
The utility of the above is apparent when used with stochastic or chaotic systems,
allowing the values to be focused in spite of their non-determinism.

#### Output Dev Roadmap: Composite slopes

In future the outputs will be composed of three elements:
1. `offset` with a dedicated `slew` time
2. `actions` with an assignable modulation added to offset
3. `pulse` functionality overlaid on all of the above

All three of these functions will be implemented as ASL slopes, composed together
in the `IO_BlockProcess()` function. This requires some small extensions in the
C-layer, but primarily requires development in the lua Output library.

#### Output Dev Roadmap: Audio Signals

Crow contains plenty of processing power to be able to implement audio-rate signal
processing. Indeed the slopes of the ASL language are audio-rate signal generators
themselves! There is a strong desire to extend crow's capabilities to cover some
rudimentary audio rate features.

ASL is implemented in a manner that isn't performant when run with breakpoints that
occur faster than ~100Hz. One avenue of audio-rate exploration would be to
reconfigure the Lua<->C interface of the ASL<->Slope libraries. The intention
would be to compile a directly executable descriptor to the C library such that
no lua interop would be required at each breakpoint. Please discuss...

Additionally (alternatively?) some kind of dynamic DSP graph implementation could
be brought to the platform such that outputs could modally switch between ASL slopes
and some yet-to-be-named audio signal generator. Inputs can of course be processed
though the input sample-rate is limited as mentioned above.

If you're intending to take this project on, consider the wide range of DSP library
functions already provided in the wrDsp library. In the spirit of the crow, it is
recommended that any such endeavours use a declarative functional style for the
user-facing element. See ASL as an example of a declarative syntax for a time-variant
process.

### I2C: Adding support for new devices
In order to encourage wide-ranging support for all i2c capable devices, crow
requires only a single file to be added per device. This lua file is a simple
declarative specification of how that device communicates on the i2c bus.

These files live in `crow/lua/ii/<device_name>.lua`

#### Beyond the obvious
Take a look at `jf.lua` as an example which adds support for mannequins'
'just friends' module. This module takes advantage of most of the existing
features of the build system.

The first 4 lines are global settings stating this module is called 'just friends',
made by 'mannequins' and can be talked to at the hexadecimal address 0x70. The
`lua_name` field *must* match the filename (jf.lua -> 'jf'), and is the name
by which users will refer to this device in their scripts eg: `ii.jf.trigger()`.

Following this header is a big table called 'commands' which is itself full of
tables, one for each 'setter' command the device can receive. A 'setter' in this
context typically allows crow to remotely-control a parameter or event. More
generally a 'setter' is any command that doesn't return any values.

- 'name' is how the user will refer to this command: eg: name=trigger -> ii.jf.trigger()
- 'cmd' is the number the remote device expects to trigger the 'name'd function. see https://github.com/monome/libavr32/blob/master/src/ii.h for a starting point.
- 'docs' is an optional string describing the functionality of the command.
- 'args' is a table of tables, each inner-table describing a command argument.

##### No arguments
In the simplest case, the command doesn't require *any* arguments, eg: ii.jf.reset()
in which case you should omit the 'args' descriptor altogether.

##### 1 argument
If the command expects a single argument you can declare it as a table directly.
eg: args = `{ 'name', type }`
It is also allowed to use the nested table syntax: `args = { { 'name', type } }`

##### 2+ arguments
The most general case allows for any number of arguments, each defined as a table
within the args table like so:
```
args = { { 'arg1', type }
       , { 'arg2', type }
       }
```

##### What's a type?!
Each argument has both a *name* and *type*. The *name* is purely for documentation, so
the user knows at a glance what that argument means. The *type* refers to the
low-level representation of the value.

Available types are:
- void  -- the lack of an argument (typically not needed)
- u8    -- unsigned 8-bit integer (0,255)
- s8    -- signed 8-bit integer (-128,127)
- u16   -- unsigned 16-bit integer (0,65535)
- s16   -- signed 16-bit integer (-32768, 32767) {teletype's default}
- float -- 32bit floating point number

*u16* and *s16* expect MSB before LSB.
*float* is little-endian

Refer to the documentation or source-code for the device you're supporting to
determine what types are used for these types.

##### Get
Many devices built for use within the teletype ecosystem have a matching 'getter' to
go with each 'setter'. So while `JF.RMODE 1` sets 'run_mode' to 1, `JF.RMODE` without
any argument, will *query* the value of 'run_mode' returning that as a result.

Under the hood, these matching getters & setters are typically given the same 'cmd'
value, with the getter offset by 0x80. The *last* argument in the list is omitted
when calling the command, and instead is expected as the return value.

By setting `get = true`, a getter will be auto-generated with the above assumptions.

If you have getters that *don't* use these conventions, you can articulate them
explicitly. >>>

##### Getters

A single function is provided for getting, or *querying*, values from a connected
device. this is: `ii.jf.get( name, ... )`. To define 'gettable' values, you add them
to the 'getters' table in much the same way as the 'commands' table.

- `name` is the name by which to query the state
- `cmd` is the number corresponding the above name (see your devices docs)
- `docs` is an optional string describing what will be returned
- `args` is identical to the 'commands' version. can be omitted if no arguments.
- 'retval' is like a single `arg`. it should be a table with a string then a type.

#### Some gotchas

To simplify the build process, the 'lua_name' variable currently must match
the name of the .lua file (ie. `jf.lua` must have `lua_name = 'jf'`)

#### I2C Roadmap: Fract types & normalization
While extensive, this descriptive framework could be extended to add any or all of
the below features. See the github issue #49 for discussion:

- remove requirement that filename match `lua_name`
- `lua_name` aliases to allow eg: 'jf' or 'justfriends' to access the same table
- additional argument & `return_value` types
- fractional types with radix. eg: fract13 converts a 13bit int to 0-1.0 float
- normalization of types to a predefined voltage range
- multiple return values (eg: `ii.jf.get('retune',1)`) could return both `num` and `denom`
