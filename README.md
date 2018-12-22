# crow
an embedded lua interpreting usb<->cv bridge (for norns).
a collaboration by whimsical raps & monome

## toolchain setup

### prereq

- fennel: `luarocks install fennel`
- _others?_

### folder layout

```
crow/
  wrLib/
  wrDsp/
  stm32f7/
    STM32_Cube_F7/
    dfu-stm32f7/
    lua/
    crow/
```

bottom folder is this repo.

recipe:

```
mkdir crow
cd crow
git clone https://github.com/whimsicalraps/wrLib
git clone https://github.com/whimsicalraps/wrDsp
mkdir stm32f7
cd stem32f7
git clone https://github.com/trentgill/dfu-stm32f7.git
git clone https://github.com/trentgill/lua.git
git clone https://github.com/trentgill/STM32_Cube_F7.git
git clone https://github.com/trentgill/crow.git
```

