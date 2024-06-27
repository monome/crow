TARGET=parafocus
EXECUTABLE=$(TARGET).elf

GIT_VERSION := $(shell git describe --tags)

CUBE=submodules/STM32CubeF7_Drivers
HALS=$(CUBE)/STM32F7xx_HAL_Driver/Src
USBD=submodules/STM32CubeF7_USB/STM32_USB_Device_Library
WRLIB=submodules/wrLib
WRDSP=submodules/wrDsp
BOOTLOADER=submodules/dfu-stm32f7
PRJ_DIR=parafocus

CC=arm-none-eabi-gcc
LD=arm-none-eabi-gcc
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump

# BIN=$(CP) -O ihex 
BIN = $(TARGET).bin

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F7XX -DARM_MATH_CM7 -DHSE_VALUE=8000000
DEFS += -DSTM32F722xx -DUSE_HAL_DRIVER
STARTUP = $(CUBE)/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f722xx.s

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
CFLAGS += -Wno-unused-function -Wno-unused-value
CFLAGS += $(MCFLAGS)
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEFS) -I. -I./ $(STM32_INCLUDES)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -fno-common
CFLAGS += -DVERSION=\"$(GIT_VERSION)\"
CFLAGS += -ffunction-sections -fdata-sections # provides majority of LTO binary size reduction

# debugger: choose between uart (=0) & swtrace(=1). latter requires hardware mod
TRACE ?= 0
ifeq ($(TRACE), 1)
	CFLAGS += -DTRACE
endif

# release: if (=1), disable all debug prints
R ?= 0
ifeq ($(R), 1)
	CFLAGS += -DRELEASE
	#CFLAGS += -flto # broken in debug mode. provides a small LTO binary size reduction
endif

LDFLAGS = -Wl,-T,stm32_flash.ld,-flto,-gc-sections
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
	$(WRLIB)/wrMeters.c \
	$(WRLIB)/wrQueue.c \
	$(WRDSP)/wrBlocks.c \
	$(WRDSP)/wrFilter.c \


# recipes!
all: $(TARGET).hex $(BIN)


# build the objects from c source
OBJDIR = .
OBJS = $(SRC:%.c=$(OBJDIR)/%.o)
OBJS += Startup.o


# C dependencies echoed into Makefile
DEP = $(OBJS:.o=.d)  # one dependency file for each source


# OS dependent size printing
UNAME := $(shell uname)

GETSIZE = stat

ifeq ($(UNAME), Darwin)
	GETSIZE = stat -x
endif


# include all DEP files in the makefile
# will rebuild elements if dependent C headers are changed
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
	@echo "        ^ must be less than 512kB"
	# 512kb -64kb(bootloader) -128kb(scripts)

flash: $(BIN)
	st-flash write $(BIN) 0x08000000

debug:
	make flash TRACE=1
	stlink-trace -c 216

dfu: $(BIN)
	sudo dfu-util -a 0 -s 0x08000000 -R -D $(BIN) -d ,0483:df11

dfureset:
	@stty -F /dev/ttyACM0 raw speed 115200
	@echo '^^b' > /dev/ttyACM0
	@sleep 1

pydfu: $(TARGET).dfu $(BIN)
	@python3 util/pydfu.py -u $<

$(TARGET).dfu: $(BIN)
	python3 util/dfu.py -D 0x0483:0xDF11 -b 0x08000000:$^ $@

boot:
	cd $(BOOTLOADER) && \
	make R=1 flash

zip: $(BIN) $(TARGET).dfu
	mkdir -p $(TARGET)-$(GIT_VERSION)
	cp util/osx_linux-update_firmware.command $(TARGET)-$(GIT_VERSION)/
	cp util/osx_linux-erase_userscript.command $(TARGET)-$(GIT_VERSION)/
	cp util/windows-update_firmware.bat $(TARGET)-$(GIT_VERSION)/
	cp util/windows-erase_userscript.bat $(TARGET)-$(GIT_VERSION)/
	cp util/blank.bin $(TARGET)-$(GIT_VERSION)/
	cp $(BIN) $(TARGET)-$(GIT_VERSION)/
	cp $(TARGET).dfu $(TARGET)-$(GIT_VERSION)/
	zip -r $(TARGET)-$(GIT_VERSION).zip $(TARGET)-$(GIT_VERSION)/

%.o: %.c
	@$(CC) -ggdb $(CFLAGS) -c $< -o $@
	@echo $@

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

%.s: %.c
	@$(CC) -ggdb $(CFLAGS) -S $< -o $@

Startup.o: $(STARTUP)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo $@

erase:
	st-flash erase

.PHONY: clean
clean:
	@rm -rf Startup.lst $(TARGET).elf.lst $(OBJS) $(AUTOGEN) \
	$(TARGET).bin  $(TARGET).out  $(TARGET).hex $(TARGET).dfu \
	$(TARGET).map  $(TARGET).dmp  $(EXECUTABLE) $(DEP) \
	$(TARGET)-$(GIT_VERSION)/  *.zip \
