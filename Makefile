# put your *.o targets here, make should handle the rest!

# SRCS = main.c stm32f4xx_it.c system_stm32f4xx.c

 #SRCS = main.c system_stm32f4xx.c diskio_sdio.c sdio_stm32f4.c ff.c pl_uart_debug.c emgr.c fsmgr.c logger_uart.c ccsbcs.c

SRCS = main.c system_stm32f4xx.c diskio_sdio.c sdio_stm32f4.c ff.c pl_uart_debug.c emgr.c fsmgr2.c logger_uart.c ccsbcs.c

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

PROJ_NAME=sddisk

# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
OPENOCD=sudo /home/garry/arm/stlink/bin/openocd

CFLAGS  = -g -O2 -Wall -Tstm32_flash.ld 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

###################################################

vpath %.c src ff
vpath %.a lib

ROOT=$(shell pwd)

CFLAGS += -Iinc -Ilib -Ilib/inc  -Isrc
CFLAGS += -Ilib/inc/core -Ilib/inc/peripherals 
CFLAGS += -Iff
# CFLAGS += -Iff/option

SRCS += lib/startup_stm32f4xx.s # add startup file to build

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj

lib:
	$(MAKE) -C lib

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -Llib -lstm32f4 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f lib/*.o
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin

upload: $(PROJ_NAME).elf 
	$(OPENOCD) -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg -c init -c targets -c "halt" -c "flash write_image erase $(PROJ_NAME).elf" -c "verify_image $(PROJ_NAME).elf" -c "reset run" -c shutdown
