# crow development setup

crow development can mean many things, but if you want to write anything beyond a user script, you'll need to setup the dev environment so you can build binaries. A binary can then be flash to crow using the DFU bootloader over USB. If you want to work on the bootloader itself you'll need an ST-Link and an edge-connector (see below).

## Toolchain setup

### Prerequisites

- [arm-none-eabi-gcc 4.9.3](https://launchpad.net/gcc-arm-embedded/4.9)
- lua 5.2 or higher
- dfu-util 0.8+ OR python3 with pyusb

### Get the project
- `git clone --recursive https://github.com/monome/crow.git`
- `cd crow`
- `git submodule update --init` *nb: will take a while to download*

### Building

- `make` build the project binary (**see nb below**)
- `make clean` remove all binary objects to force a rebuild on next `make`

I always add the `-j` flag to `make` so it runs multi-threaded (as the compile time is over 10s on my machine). So run `make -j` for fastest build time.

### Docker

You can get a build environment with
[Docker](https://docs.docker.com/) on any platform as follows. The
`--config core.autocrlf` setting may be necessary on Windows to deal
with line ending issues when mounting the directory into the Docker
container.

```bash
git clone --recursive --config core.autocrlf=input https://github.com/monome/crow
cd crow
docker build . -t crow-dev
docker run --rm -it -v "$(pwd)":/target --entrypoint /bin/bash crow-dev
```

and then `make` to build, or use `docker run --rm -it -v
"$(pwd)":/target crow-dev` for the container to run `make` and then
exit.


#### DFU Programmer

Before flashing with DFU, crow must be put into bootloader mode. That can be acheived with any of the following:
* send `^^b` to crow from druid or similar
* `make dfureset` *linux only, uses `stty`, assumes crow is on /dev/ttyACM0*
* 'force' the bootloader by bridging i2c pins and resetting crow

There are 2 options for DFU programming crow:

* dfu-util: `make dfu`
* pydfu: `make pydfu`

This will build the project and then attempt to flash it over the crow USB port.

`pydfu` is preferred as it has faster upload speeds, and less ambiguous error messaging.

#### ST-link / Discovery board

Using an st-link, or stm discovery board as the programmer is also possible but you'll need:
- st-link, stm discovery board, or compatible programmer
- 'edge piece' [pcb](https://oshpark.com/shared_projects/lMkbP9a9)
- [edge connector](https://www.digikey.com/product-detail/en/7-5530843-7/A101966-ND/2310829)
- 6pin male [header](https://www.digikey.com/product-detail/en/sullins-connector-solutions/PREC006SFAN-RC/S1212EC-06-ND/2774968)

Or if you're adventurous, you can get by without the pcb and just wire a 2x3 header directly to the edge connector.

Pinout for SWD port on discovery board (and thus on the edge-piece 1x6pin connector):
1. VDD target (ie 3v3 from crow)
2. SWD clock
3. Ground
4. SWD data i/o
5. !Reset
6. SWO (not connected on crow)

The edge connector pin out is:
- top-side starting at rear-edge: TRACE, SWD clock, !RST
- bottom-side starting at rear-edge: 3v3, SWD data, Ground

Once you've got the hardware sorted, you can:
- `make boot` build & flash the bootloader
- `make flash` build & flash the main program
- `make erase` erase the whole program including calibration metadata & userscript

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

1. Run the `util/ii_gen.lua` script to autogenerate i2c links of anything in `lua/ii/*` -> results put into `build/*`
2. Wrap all remaining `.lua` files in `lua/*` and `build/*` into c-headers with the `l2h.lua` script
3. Build the C objects, including the lua VM which is built directly into the binary
4. Run the linker and create the binary

Then you can upload to crow with `make dfu` or `make flash`.

If you want to improve the way lua code is included in the binary see issue [#40](https://github.com/monome/crow/issues/40).

## Making a Release

Once new changes are tested & ready for production, follow these steps to provide a new release:

* `git merge` all changes to `main` branch (if there's conflicts, something went wrong & needs more testing). This step can also be done from within github if preferred.
* (optional) `make` to ensure there's no errors or warnings
* edit `version.txt` with new semantic version & webaddress (you can make a github draft release if needed)
* `git commit -m "release X.X.X"`
* `git tag vX.X.X` with semantic version
* `git push` uploads the changes
* `git push --tags` uploads tags (i think this also does a plain `git push` but am unsure)
* `make clean` so new tag will take effect
* `make zip` to build all binaries and create the zip archive
* Make a github Release & upload `crow-vX.X.X.zip` AND `crow.dfu`. Describe the release & manually add Changelog
* (probably) make a new lines thread announcing the release (provides forum for feedback)

## Project structure

- `ll/*`: all the low-level hardware drivers written in C using STM32 HAL.
- `usbd/*`: USB device driver for TTY communication to host.
- `lib/*`: hardware-agnostic C APIs to the LL drivers & system functions.
- `lua/*`: crow system providing lua APIs to the platform.
- `lua/ii/*`: descriptor files for supported i2c devices.

- `util/*`: helpers for the build process
- `tests/*`: a few tests for the lua scripts
- `build/`: a temporary folder for collecting generated sources

### Linking C functions and Lua

The file `crow/lib/lualink.c` contains the interface between the C infrastructure
and lua runtime. If you need to extend this interface, a wrapper function should
be created and linked in the `libCrow` struct.

Functions calling from Lua to C must follow the function prototype:
`static int _function_name( lua_State* L );`

Lead your C function with an underscore, and see the Lua docs for how to pass
arguments to and from these functions.

Callbacks to Lua are defined at the end of this file, by convention named
`L_handle_eventname` where 'eventname' is your callback's identifier. Each of these
callbacks is called through the *event* system via a function `L_queue_eventname`.

Your C-code should only call the the `L_queue*` functions and pipe them through via
the `crow/lib/events.*` system.

### Including Lua files in the binary

If you add a new `*.lua` file to the project at `lua/*.lua` you'll need to include
it in `lib/lualink.c` so as to have it built into the binary:

When these files are included in the project, their filenames are manipulated
automatically, and they're wrapped into long string arrays. Thus if you have a file
at `lua/example.lua` it will be wrapped into `lua/example.lua.h` and will contain a
single `char*` array with the name `lua_example` containing the whole lua file
as raw text.

To include it in the binary add an `#include` at the top of the file with `.h`
appended to your `*.lua` filename (ie. `*.lua.h`).

To then allow lua's `dofile()` to find this code, you need to add a link to
the struct `Lua_libs` immediately after the includes.
    This should contain a string representing the path to the file, where `/` is
    replaced by `_` and the file extension is dropped. Followed by the array
    identifier, identical to the string, but without quotes.

Nb: this does *not* apply to i2c descriptors. They are automatically included!

### Flash layout

Crow has 512kB of Flash memory which is segmented as follows:
- 48kB, DFU bootloader (0x08000000)
- 16kB, calibration data (0x0800C000)
- 64kB, user-script (0x08010000)
- 384kB, crow application (0x08020000)

The *DFU bootloader* is protected and isn't accessible from the main program.
Changing the bootloader requires a hardware programmer (see above).

The *calibration data* section contains automatic calibration information calculated
when the unit is first programmed.

The *user-script* location is where a crow script can be uploaded to. At present
this is simply a lua program as raw text, offset by a single 32bit word. The leading
word contains the length of the user-script in bytes, plus a status byte so the
system can query whether there is currently a program loaded. If no program is found
the system will load the script at `lua/default.lua`.

The *crow application* section includes all of the crow standard libraries in lua,
along with the C application.

Further technical details in `lib/flash.h/c`

### The REPL

Crow communicates with a host over a serial port. The C API is in `lib/caw.c` along
with parsing of *crow commands* (eg. `^^s`).

The parser tries to capture complete lua chunks which are queued for later processing.
The results of the parsing are handled in `main.c`'s `while(1)` loop.

Received strings are forwarded through the `REPL_eval()` function which chooses
between 2 functions depending on the REPL state:
1. Forwards the details to `Lua_eval()` to be executed immediately.
2. Forwards to `REPL_receive_script()` to save the incoming text to a buffer.

In the first case, the incoming serial stream is expected to be a well-formed lua
script and is called directly with `lua_pcall()`. If the script contains errors
they will be reported back over the USB port with whatever info is available.

Switching between modes is accomplished by sending a *crow command* alerting the
device that, until further notice, the following serial data should be saved for
later use as a full lua script. Once the full script is sent, the user ends with
another *crow command*. On completion of this sequence a number of things happen:
- crow is restarted to stop the currently running script
- serial stream is validated for correct lua syntax
- lua code is loaded with `lua_pcall()` again checking for errors
- if using `^^w` (not `^^e`) stream is saved into flash memory
- temporary buffer holding the script is destroyed
- REPL switches back to direct mode, where it will interpret incoming text directly

#### Communication details

If you're writing a host environment to control/upload to crow, there are some
limitations to keep in mind:

The usb connection will break up any messages into 64byte blocks. If you want
to send a codeblock longer than this, you need to use the multiline helper.
first send a command of 3 backticks "```", then your codeblock, then another
3 backticks. This should be hidden from the end-user. This is limited to 2kB.

For longer chunks where you're uploading a whole script, use the `^^s <code> ^^e` sequence to run immediately. Alternatively, use `^^s <code> ^^w` if you want to write the script to flash so it persists across power cycles. The code block is limited to 8kB.

There is a known bug on OSX & Windows machines where packets with length equal to a multiple of 64bytes will kill the USB connection. Before sending a packet over USB, check if (length % 64 == 0) and if so, add a trailing newline (`\n`) character to the stream.

### Signals

The signal i/o for the input and output jacks happens in the `lib/io.c` file in
`IO_BlockProcess()`. Signals are generated at ~48kHz. This function processes a
block of samples per invocation. The number of samples is available in `b->size`
and is currently 32samples as of this writing.

All signals are processed as floating point numbers representing voltages directly.
Thus the range is [-5.0, 10.0] and anything beyond these values will be clipped to
the maximum when reaching the output driver.

The `IO_block_t` type is a struct described in `ll/adda.h` and should be used to
access the signals and metadata in the block processor.

In general, signal processing extensions should be built in C and provide declarative-style hooks to the lua environment. As the signal processing runs at higher priority than the lua environment, this ensures that signal outputs don't run into underruns due to high CPU usage. Accompanying lua libraries to provide lua-style syntax is encouraged (see metro.lua or input.lua for examples).

#### Inputs

The input jacks are only sampled once per block, but the `b->in[channel][]` array
is linearly interpolated from block to block. Instantaneous input values are also
available directly from the low-level driver to avoid this smoothing, and for access
without having to change the block process with `IO_GetADC(channel)`.

The block process passes the input buffers to the Detect library implemented in
`lib/detect.c` which analyse the input for changes that should generate events in
the lua environment. Event descriptors are sent from the lua Input library, which
define what will cause event interrupts to occur.


#### Input Dev Roadmap: More detection modes

The principle area of extension for the Input libraries is in the Detect library.

Additional modes will be added specifically covering:
- pitch detection for implementing tuners / volt-per-octave calibrators
- amplitude event detection for triggering events based on audio inputs


#### Outputs

Outputs are generated directly from the Slope library in `lib/slope.c` and again
the configuration of these slope generators is controlled from lua, via ASL
descriptors.

The output vector is generated by the `S_step_v()` function and will handle
breakpoints (ie. when the slope reaches it's destination) by calling into lua to
request the next slope description. Note this request will not be returned until
the following DSP block due to the event system. See Issue [#112](https://github.com/monome/crow/issues/112).

#### Output Dev Roadmap: Composite slopes

In future the outputs should be composed of three elements:
1. `offset` with a dedicated `slew` time
2. `actions` with an assignable modulation added to offset
3. `pulse` functionality overlaid on all of the above

All three of these functions will be implemented as ASL slopes, composed together
in the `IO_BlockProcess()` function.

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

### I2C: Adding support for new devices
In order to encourage wide-ranging support for all i2c capable devices, crow
requires only a single file to be added per device. This lua file is a
declarative specification of how that device communicates on the i2c bus.

These files live in `crow/lua/ii/<device_name>.lua`

#### Beyond the obvious
Take a look at `jf.lua` as an example which adds support for Mannequins'
'Just Friends' module. This module takes advantage of most of the existing
features of the build system.

The first 4 lines are global settings stating this module is called 'just friends',
made by 'mannequins' and can be talked to at the hexadecimal address 0x70. The
`lua_name` field *must* match the filename (jf.lua -> 'jf'), and is the name
by which users will refer to this device in their scripts eg: `ii.jf.trigger()`.

*NEW in v1.0.3*: `i2c_address` may now be a table of addresses to allow automatic support for devices that support multiple units by using different i2c addresses.
eg txI: `address = {0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F}`
Note that an enumerate list is required (not just a range), and the should be listed in the order that users will address them. In the case of txI, `ii.txi[1]` will refer to address `0x68`.

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
- s16V  -- voltage as signed 16-bit integer (-32768, 32767) {converts teletype to V}
- float -- 32bit floating point number

*u16*, *s16*, and *s16V* expect MSB before LSB.
*float* is little-endian.

Note *s16V* is used to bridge from teletype native 16bit integers, and crow's
floating point voltage representation. If you typically use `N` or `V` teletype
operators on that parameter, you want this!

Refer to the documentation or source-code for the device you're supporting to
determine what types are used for each argument.

##### Get
Many devices built for use within the teletype ecosystem have a matching 'getter' to
go with each 'setter'. So while `JF.RMODE 1` sets 'run_mode' to 1, `JF.RMODE` without
any argument, will *query* the value of 'run_mode' returning that as a result.

Under the hood, these matching getters & setters are typically given the same 'cmd'
value, with the getter offset by 0x80. The *last* argument in the list is omitted
when calling the command, and instead is expected as the return value.

By adding `get = true` to the setter table, a getter will be auto-generated with the
above assumptions.

If you have getters that *don't* use these conventions, you can articulate them
explicitly. >>>

##### Getters

A single function is provided for getting, or *querying*, any and all values from a
connected device. eg: `ii.jf.get( name, ... )`. To define 'gettable' values, you
add them to the 'getters' table in much the same way as the 'commands' table.

- `name` is the name by which to query the state
- `cmd` is the number corresponding the above name (see your devices docs)
- `docs` is an optional string describing what will be returned
- `args` is identical to the 'commands' version. can be omitted if no arguments.
- 'retval' is like a single `arg`. it should be a table with a string then a type.

#### Some gotchas

To simplify the build process, the 'lua_name' variable currently must match
the name of the .lua file (ie. `jf.lua` must have `lua_name = 'jf'`)

#### Supporting custom protocols with the `pickle` and `unpickle` params
While the i2c framework in crow is quite elegant, there are a few edge cases & pre-existing devices that just don't fit with the assumptions. To handle these situations, you can use the `pickle` and `unpickle` keys to write custom handlers.

The idea is to provide a low-level translation layer from crow's internal i2c representation, into the required bytes on the i2c line. In general, you should keep the i2c descriptor file as similar to the others as possible.

For example, on Telex-I, when crow queries it for a value, it expects the command (what input to grab) and the choice of input to be sent in a single byte. We emulate this by allowing the i2c framework to treat them separately- 1 parameter for command, and a separate parameter for channel, then provide a `pickle` and `unpickle` function to do the dirty work of pushing the two bytes together.

```lua
, pickle = -- combine command & channel into a single byte & set address
--void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
[[

uint8_t chan = data[1] - 1;  // zero index
data[0] |= (chan & 0x3);     // mask channel
*byte_count = 1;             // packed into a single byte
*address += chan >> 2;       // ascending vals increment address

]]
, unpickle = -- using the same command to parse the response from any channel
-- void unpickle( uint8_t* address, uint8_t* command, uint8_t* data );
[[

*command &= ~0x3;  // use same command for all 4 channels (by discarding 2LSBs)

]]
```

* Everything between the `[[` and `]]` is a multi-line string in Lua
* The code block in the middle is written in C (see below for fn signature)
* `pickle` is for crow -> i2c, while `unpickle` is i2c -> crow.

The C function signatures for pickle and unpickle are like this, and it's a good idea to include them in the descriptor for reference if you're using them:
```C
void pickle( uint8_t* address, uint8_t* data, uint8_t* byte_count );
void unpickle( uint8_t* address, uint8_t* command, uint8_t* data );
```

In general, these functions should only be used if the serial protocol isn't working correctly, or there's some kind of low-level hack required to satisfy an existing device. If at all possible, try to avoid using them, and if you're designing a new device, please use the standard protocol!


#### I2C Roadmap: Descriptor improvements
While extensive, this descriptive framework could be extended to add any or all of
the below features. See the github issue #49 for discussion:

- remove requirement that filename match `lua_name`
- `lua_name` aliases to allow eg: 'jf' or 'justfriends' to access the same table
- additional argument & `return_value` types
- multiple return values (eg: `ii.jf.get('retune',1)`) could return both `num` and `denom`
- support for per-module lua libraries that are loaded along with the ii bindings when used.

### I2C Roadmap: Follower mode

At it's most basic, crow can be treated as a simple expander for the i2c bus.
It provides 2 inputs & 4 outputs to extend teletype's IO capabilities. To use these
no changes are required to the default setup on crow. Simply use:

`CROW.IN a` where a is 1 or 2. crow will return the current value of that input.
`CROW.OUT a b` setting output 'a' to the value 'b'
`CROW.SLEW a b` setting the slew rate of output 'a' to the time 'b'
`CROW.PULSE a` performs a pulse on output 'a'

crow has default actions to handle these messages, though like most things in crow
they can be redefined for our own purposes by editing the functions in the table
`ii._c`. Try printing the follower-help with `ii._c.help()` for a list of functions
that can be redefined.

#### A crow call

crow is capable of far more than reporting the state of its inputs and setting the
output values, but so vast are the possibilities that we couldn't make an i2c
command for every one! To deal with this flexibility, we 'CALL' to crow and define
the expected function on crow itself.

eg: I want teletype to be able to add a voltage to a given output. There's no way
to query the state of crow output via i2c, so we'll need to do it natively on crow
itself. Something like:

```
function add_to_output( channel, amount_to_add )
    output[channel].offset = output[channel].offset + amount_to_add
end
```

To execute the above from i2c we use one of the 'CALL' functions, in this case
`CALL2` as we need to send 2 arguments. There are commands for 1-4 arguments. From
teletype:
`CROW.CALL2 1 V 1`
This should add 1 volt to the first output jack on crow.

We can then redefine the function at `ii._c.call2()` to call our `add_to_output()`
function.

#### Calling with context

The above function is great when you just want to add a single additional function,
but what about if you want to do a number of things that all need 1 argument.
In this case you can use `CALL` with 1 extra argument than your function needs, but
use the first number to choose which function to execute.

*nb: unfortunately, i don't think we can index this table after naming the functions.*

```
local actions=
{ add_an_octave = function(arg) add_to_output(arg, octave(1.0)) end
, add_a_fifth   = function(arg) add_to_output(arg, semitone(7/12)) end
, add_random    = function(arg) add_to_output(arg, semitone(Math.rand())) end
}

ii._c.call2 = function(cmd, arg2)
    actions[cmd](arg2)
end
```

Then on teletype:
`CROW.CALL2 1 1`  adds an octave to output 1
`CROW.CALL2 1 2`  adds an octave to output 2
`CROW.CALL2 2 1`  add a fith to output 1
`CROW.CALL2 3 4`  move output 4 by a random number of semitones
