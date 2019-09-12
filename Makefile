TARGET=crow
EXECUTABLE=$(TARGET).elf

GIT_VERSION := $(shell git describe --tags)

CUBE=submodules/STM32_Cube_F7/Drivers
HALS=$(CUBE)/STM32F7xx_HAL_Driver/Src
USBD=submodules/STM32_Cube_F7/Middlewares/ST/STM32_USB_Device_Library
WRLIB=submodules/wrLib
WRDSP=submodules/wrDsp
LUAS=submodules/lua/src
BOOTLOADER=submodules/dfu-stm32f7
BUILD_DIR := build
PRJ_DIR=crow

CC=arm-none-eabi-gcc-4.9.3
LD=arm-none-eabi-gcc-4.9.3
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump

FENNEL=fennel

# BIN=$(CP) -O ihex 
BIN = $(TARGET).bin

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F7XX -DARM_MATH_CM7 -DHSE_VALUE=8000000
DEFS += -DSTM32F722xx -DUSE_HAL_DRIVER
STARTUP = $(CUBE)/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f722xx.s

# MCFLAGS = -march=armv4e-m -mthumb 
# MCFLAGS = -mthumb -march=armv4e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16
MCFLAGS = -mthumb -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16

STM32_INCLUDES = \
	-I$(WRLIB)/ \
	-I$(WRDSP)/ \
	-I$(CUBE)/CMSIS/Device/ST/STM32F7xx/Include/ \
	-I$(CUBE)/CMSIS/Include/ \
	-I$(CUBE)/STM32F7xx_HAL_Driver/Inc/ \
	-I/usr/local/include/ \
	-Iusbd/ \
	-I$(USBD)/Class/CDC/Inc/ \
	-I$(USBD)/Core/Inc/ \

OPTIMIZE       = -O2

CFLAGS += -std=c99
CFLAGS += -Wall
CFLAGS += -Wno-unused-function
CFLAGS += $(MCFLAGS)
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEFS) -I. -I./ $(STM32_INCLUDES)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -DLUA_32BITS -DLUA_COMPAT_5_2
CFLAGS += -fno-common
CFLAGS += -DVERSION=\"$(GIT_VERSION)\"

# debugger: choose between uart (=0) & swtrace(=1). latter requires hardware mod
TRACE ?= 0
ifeq ($(TRACE), 1)
	CFLAGS += -DTRACE
endif

# release: if (=1), disable all debug prints
R ?= 0
ifeq ($(R), 1)
	CFLAGS += -DRELEASE
endif


LDFLAGS = -Wl,-T,stm32_flash.ld
LIBS = -lm -lc -lnosys

SRC = main.c \
	stm32f7xx_it.c \
	system_stm32f7xx.c \
	$(HALS)/stm32f7xx_hal.c \
	$(HALS)/stm32f7xx_hal_cortex.c \
	$(HALS)/stm32f7xx_hal_rcc.c \
	$(HALS)/stm32f7xx_hal_rcc_ex.c \
	$(HALS)/stm32f7xx_hal_flash.c \
	$(HALS)/stm32f7xx_hal_flash_ex.c \
	$(HALS)/stm32f7xx_hal_gpio.c \
	$(HALS)/stm32f7xx_hal_i2c.c \
	$(HALS)/stm32f7xx_hal_i2s.c \
	$(HALS)/stm32f7xx_hal_dma.c \
	$(HALS)/stm32f7xx_hal_dma2d.c \
	$(HALS)/stm32f7xx_hal_pcd.c \
	$(HALS)/stm32f7xx_hal_pcd_ex.c \
	$(HALS)/stm32f7xx_hal_pwr.c \
	$(HALS)/stm32f7xx_hal_pwr_ex.c \
	$(HALS)/stm32f7xx_hal_rng.c \
	$(HALS)/stm32f7xx_hal_spi.c \
	$(HALS)/stm32f7xx_hal_tim.c \
	$(HALS)/stm32f7xx_hal_tim_ex.c \
	$(HALS)/stm32f7xx_hal_uart.c \
	$(HALS)/stm32f7xx_hal_usart.c \
	$(HALS)/stm32f7xx_ll_usb.c \
	$(wildcard lib/*.c) \
	$(wildcard ll/*.c) \
	$(wildcard usbd/*.c) \
	$(USBD)/Core/Src/usbd_core.c \
	$(USBD)/Core/Src/usbd_ctlreq.c \
	$(USBD)/Core/Src/usbd_ioreq.c \
	$(USBD)/Class/CDC/Src/usbd_cdc.c \
	$(WRLIB)/str_buffer.c \
	$(WRLIB)/wrConvert.c \
	$(WRLIB)/wrMath.c \


# lua tests
LTESTS = $(wildcard tests/*.lua) \


# recipes!
all: $(TARGET).hex $(BIN)


### pre-process only files

# fennel script conversion to lua
FNL_SRC = $(wildcard util/*.fnl) \
	$(wildcard lua/*.fnl) \

FNL_PP = $(FNL_SRC:%.fnl=%.lua)

# i2c descriptors
II_SRCD = lua/ii
II_SRC = $(wildcard $(II_SRCD)/*.lua)
II_TARGET = $(addprefix $(BUILD_DIR)/ii_, $(notdir $(II_SRC)))

$(II_TARGET): util/ii_lua_module.lua

$(BUILD_DIR)/ii_%.lua: $(II_SRCD)/%.lua util/ii_lua_module.lua | $(BUILD_DIR)
	@lua util/ii_lua_module.lua $< $@
	@echo lua $@

$(BUILD_DIR)/iihelp.lua: $(II_SRC) util/ii_lua_help.lua | $(BUILD_DIR)
	@lua util/ii_lua_help.lua $(II_SRCD) $@
	@echo lua $@

$(BUILD_DIR)/ii_c_layer.h: $(II_SRC) util/ii_c_layer.lua | $(BUILD_DIR)
	@lua util/ii_c_layer.lua $(II_SRCD) $@
	@echo lua $@

$(BUILD_DIR)/ii_lualink.h: $(II_SRC) util/ii_lualinker.lua | $(BUILD_DIR)
	@lua util/ii_lualinker.lua $(II_SRCD) $@
	@echo lua $@


### destination sources

# lua srcs: these get converted to bytecode strings wrapped in c-headers
LUA_SRC  = $(wildcard lua/*.lua)
LUA_SRC += $(BUILD_DIR)/iihelp.lua
LUA_SRC += $(II_TARGET)

LUA_PP = $(LUA_SRC:%.lua=%.lua.h)
LUA_PP: $(LUA_SRC)

LUACORE_OBJS=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
		lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
		ltm.o lundump.o lvm.o lzio.o
LUALIB_OBJS=	lauxlib.o lbaselib.o lbitlib.o lcorolib.o ldblib.o liolib.o \
		lmathlib.o loslib.o lstrlib.o ltablib.o lutf8lib.o loadlib.o linit.o


# build the objects from c source
OBJDIR = .
OBJS = $(SRC:%.c=$(OBJDIR)/%.o)
OBJS += $(addprefix $(LUAS)/,$(LUACORE_OBJS) $(LUALIB_OBJS) )
OBJS += Startup.o

# specific objects that require built dependencies (II)
$(OBJDIR)/lib/lualink.o: $(LUA_PP) $(BUILD_DIR)/ii_lualink.h
$(OBJDIR)/lib/ii.o: $(BUILD_DIR)/ii_c_layer.h

# generate the build directory
$(BUILD_DIR):
	@echo build_dir $(BUILD_DIR)/
	@mkdir -p $(BUILD_DIR)


# C dependencies echoed into Makefile
DEP = $(OBJS:.o=.d)  # one dependency file for each source


# OS dependent size printing
UNAME := $(shell uname)

GETSIZE = stat

ifeq ($(UNAME), Darwin)
	GETSIZE = stat -x
endif


.PHONY: tests
tests:
	@for t in $(LTESTS); do \
		lua $$t; \
	done

# include all DEP files in the makefile
# will rebuild elements if dependent C headers are changed
# FIXME: currently causes compiler warning due to missing .lua.h files
-include $(DEP)

$(TARGET).hex: $(EXECUTABLE)
	@$(CP) -O ihex $^ $@

$(EXECUTABLE): $(OBJS)
	@$(LD) -g $(MCFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	@echo "linked:       $@"
	@$(OBJDUMP) --disassemble $@ > $@.lst
	@echo "disassembly:  $@.lst"

$(BIN): $(EXECUTABLE)
	@$(CP) -O binary $< $@
	@echo "binary:       $@"
	@$(OBJDUMP) -x --syms $< > $(addsuffix .dmp, $(basename $<))
	@echo "symbol table: $@.dmp"
	@echo "Release: "$(R)
	@$(GETSIZE) $(BIN) | grep 'Size'
	@echo "        ^ must be less than 384kB (384,000)"
	# 512kb -64kb(bootloader) -128kb(scripts)

flash: $(BIN)
	st-flash write $(BIN) 0x08020000

debug:
	make flash TRACE=1
	#st-flash write $(BIN) 0x08020000
	stlink-trace -c 216

dfu: $(BIN)
	sudo dfu-util -a 0 -s 0x08020000 -R -D $(BIN) -d ,0483:df11

boot:
	cd $(BOOTLOADER) && \
	make R=1 flash

zip: $(BIN)
	mkdir -p $(TARGET)-$(GIT_VERSION)
	cp util/flash.sh $(TARGET)-$(GIT_VERSION)/
	cp util/erase_userscript.sh $(TARGET)-$(GIT_VERSION)/
	cp util/blank.bin $(TARGET)-$(GIT_VERSION)/
	cp $(BIN) $(TARGET)-$(GIT_VERSION)/
	zip -r $(TARGET)-$(GIT_VERSION).zip $(TARGET)-$(GIT_VERSION)/

%.o: %.c
	@$(CC) -ggdb $(CFLAGS) -c $< -o $@
	@echo $@

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.s: %.c
	@$(CC) -ggdb $(CFLAGS) -S $< -o $@

%.lua: %.fnl
	@echo f2l $< "->" $@
	@$(FENNEL) --compile $< > $@

%.lua.h: %.lua util/l2h.lua
	@echo l2h $< "->" $@
	@lua util/l2h.lua $<

Startup.o: $(STARTUP)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo $@

wav: fsk-wav

fsk-wav: $(BIN)
	export PYTHONPATH=$$PYTHONPATH:'.' && \
	cd .. && python stm-audio-bootloader/fsk/encoder.py \
		-s 48000 -b 16 -n 8 -z 4 -p 256 -g 16384 -k 1800 \
		$(PRJ_DIR)/$(BIN)

erase:
	st-flash erase

.PHONY: clean
clean:
	@rm -rf Startup.lst $(TARGET).elf.lst $(OBJS) $(AUTOGEN) \
	$(TARGET).bin  $(TARGET).out  $(TARGET).hex \
	$(TARGET).map  $(TARGET).dmp  $(EXECUTABLE) $(DEP) \
	$(BUILD_DIR) lua/*.lua.h util/l2h.lua \
	$(TARGET)-$(GIT_VERSION)/  *.zip \

splint:
	splint -I. -I./ $(STM32_INCLUDES) *.c
