TARGET=main
EXECUTABLE=main.elf

CUBE=../STM32_Cube_F4/Drivers
HALS=$(CUBE)/STM32F4xx_HAL_Driver/Src
WRLIB=../../wrLib
WRDSP=../../wrDsp
LUAS=../../lua/src
PRJ_DIR=crow

CC=arm-none-eabi-gcc-4.9.3
LD=arm-none-eabi-gcc-4.9.3
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump

# BIN=$(CP) -O ihex 
BIN = $(TARGET).bin

DEFS = -DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DARM_MATH_CM4 -DHSE_VALUE=8000000
STARTUP = $(CUBE)/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f401xe.s

# MCFLAGS = -march=armv4e-m -mthumb 
# MCFLAGS = -mthumb -march=armv4e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16
MCFLAGS = -mthumb -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16

STM32_INCLUDES = \
	-I$(WRLIB)/ \
	-I$(WRDSP)/ \
	-I$(CUBE)/CMSIS/Device/ST/STM32F4xx/Include/ \
	-I$(CUBE)/CMSIS/Include/ \
	-I$(CUBE)/STM32F4xx_HAL_Driver/Inc/ \
	-I/usr/local/include/ \
	-Iusbd/ \

OPTIMIZE       = -O2

CFLAGS += -std=c99
CFLAGS += -Wall
CFLAGS += $(MCFLAGS)
CFLAGS += $(OPTIMIZE)
CFLAGS += $(DEFS) -I. -I./ $(STM32_INCLUDES)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
#CFLAGS += -DLUA_32BITS -DLUA_COMPAT_5_2

R ?= 0
ifeq ($(R), 1)
    CFLAGS += -DRELEASE
else
    CFLAGS += -DDEBUG
endif

LDFLAGS = -Wl,-T,stm32_flash.ld
LIBS = -lm -lc -lnosys

SRC = main.c \
	stm32f4xx_it.c \
	system_stm32f4xx.c \
	$(HALS)/stm32f4xx_hal.c \
	$(HALS)/stm32f4xx_hal_cortex.c \
	$(HALS)/stm32f4xx_hal_rcc.c \
	$(HALS)/stm32f4xx_hal_rcc_ex.c \
	$(HALS)/stm32f4xx_hal_flash.c \
	$(HALS)/stm32f4xx_hal_flash_ex.c \
	$(HALS)/stm32f4xx_hal_gpio.c \
	$(HALS)/stm32f4xx_hal_i2c.c \
	$(HALS)/stm32f4xx_hal_dma.c \
	$(HALS)/stm32f4xx_hal_dma2d.c \
	$(HALS)/stm32f4xx_hal_spi.c \
	$(HALS)/stm32f4xx_hal_tim.c \
	$(HALS)/stm32f4xx_hal_tim_ex.c \
	$(HALS)/stm32f4xx_hal_usart.c \
	$(wildcard lib/*.c) \
	$(WRLIB)/str_buffer.c \
	$(WRLIB)/wrConvert.c \
	$(WRLIB)/wrMath.c \
	#$(HALS)/stm32f4xx_hal_tim_ex.c \

LUACORE_OBJS=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
	lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
	ltm.o lundump.o lvm.o lzio.o
LUALIB_OBJS=	lauxlib.o lbaselib.o lbitlib.o lcorolib.o ldblib.o liolib.o \
	lmathlib.o loslib.o lstrlib.o ltablib.o lutf8lib.o loadlib.o linit.o

OBJDIR = .
OBJS = $(SRC:%.c=$(OBJDIR)/%.o) #$(addprefix $(LUAS)/,$(LUACORE_OBJS) $(LUALIB_OBJS) )
OBJS += Startup.o

# C dependencies echoed into Makefile
DEP = $(OBJS:.o=.d)  # one dependency file for each source

# OS dependent size printing
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	GETSIZE = stat -x
endif

ifeq ($(UNAME), Linux)
	GETSIZE = stat
endif

all: $(TARGET).hex $(BIN)

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
	- $(GETSIZE) main.bin | grep 'Size'
	@echo "        ^ must be less than 320kB (320,000)"

flash: $(BIN)
	st-flash write $(BIN) 0x08000000

dfu: $(BIN)
	dfu-util -s 0x08010000 -D $(BIN) -S FFFFFFFEFFFF

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

wav: fsk-wav

fsk-wav: $(BIN)
	export PYTHONPATH=$$PYTHONPATH:'.' && \
	cd .. && python stm-audio-bootloader/fsk/encoder.py \
		-s 48000 -b 16 -n 8 -z 4 -p 256 -g 16384 -k 1800 \
		$(PRJ_DIR)/$(BIN)

erase:
	st-flash erase

clean:
	@rm -f Startup.lst $(TARGET).elf.lst $(OBJS) $(AUTOGEN) \
	$(TARGET).bin  $(TARGET).out  $(TARGET).hex \
	$(TARGET).map  $(TARGET).dmp  $(EXECUTABLE) $(DEP)

splint:
	splint -I. -I./ $(STM32_INCLUDES) *.c
