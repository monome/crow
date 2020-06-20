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

For a full walkthrough of Standalone operation with *druid*, see the [Scripting Tutorial](https://monome.org/docs/crow/scripting/), and the [Crow Reference](https://monome.org/docs/crow/reference/).
