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
simply send a 3 char command for brevity & speed

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
3 backticks. this is just like markdown.

however! if you are sending a full lua script and it's over 1kB, it must
be broken up into smaller chunks. if you are in script-transmission-mode then
you can arbitrarily break up the message every 1kB. if you're sending a giant
codeblock directly to the lua repl, you'll need to split the message so the
lua interpreter sees each portion as valid code, as each multi-line segment
is evaluated after reception.
