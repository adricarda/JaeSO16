#compiler and linker
PREFIX = arm-none-eabi
CC = $(PREFIX)-gcc
LD = $(PREFIX)-ld

#if you have not installed uarm in default location, set UARM_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/uarm),)
	UARM_DIR_PREFIX = /usr
     else
        UARM_DIR_PREFIX = /usr/local
     endif

# name of .uarm file
TARGET := p2test

#directories
SOURCES := ./src ./incl ./lib
INCLUDE = -I/usr/include/uarm -I./incl

#Compiler and linker flags
CFLAGS := -mcpu=arm7tdmi $(INCLUDE)
LFLAGS := -T $(UARM_DIR_PREFIX)/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x
LARGS :=  $(UARM_DIR_PREFIX)/include/uarm/libuarm.o $(UARM_DIR_PREFIX)/include/uarm/crtso.o

# List of source file to compile 
C_FILES	:= $(shell find $(SOURCES) -type f -name '*.c')
OBJ_FILES := $(patsubst %.c,%.o,$(C_FILES))

#Rules
all: $(TARGET).uarm

# Rules for uarm file
%.uarm: %.elf
	@echo Generating .uarm file...
	elf2uarm -k $<
	@echo Done.

# Rules for ELF file
$(TARGET).elf: $(OBJ_FILES)
	@echo Creating .elf file $@...
	$(LD) $(LFLAGS) -o $@ $(LARGS) $^

# Rules for object files

headers = $(wildcard ./incl/*.h)
%.o: %.c $(headers)
	@echo Creating object file $@...
	$(CC) -c  $(CFLAGS) -o $@ $<

clean:
	@echo cleaned
	@rm -f $(TARGET).elf
	@rm -f $(TARGET).elf.core.uarm
	@rm -f $(TARGET).elf.stab.uarm
	@rm -f $(OBJ_FILES)

