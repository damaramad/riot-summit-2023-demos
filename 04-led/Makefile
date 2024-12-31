# Copyright (C) 2024 Université de Lille
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

PREFIX          = arm-none-eabi-
CC              = $(PREFIX)gcc
LD              = $(PREFIX)gcc
OBJCOPY         = $(PREFIX)objcopy

CFLAGS          = -Wall
CFLAGS         += -Wextra
CFLAGS         += -Werror
CFLAGS         += -mthumb
CFLAGS         += -mcpu=cortex-m4
CFLAGS         += -mfloat-abi=hard
CFLAGS         += -mfpu=fpv4-sp-d16
CFLAGS         += -msingle-pic-base
CFLAGS         += -mpic-register=sl
CFLAGS         += -mno-pic-data-is-text-relative
CFLAGS         += -fPIC
CFLAGS         += -ffreestanding
ifdef DEBUG
CFLAGS         += -Og
CFLAGS         += -ggdb
else
CFLAGS         += -Os
endif
CFLAGS         += -Wno-unused-parameter
CFLAGS         += -Istdriot

LDFLAGS         = -nostartfiles
LDFLAGS        += -nodefaultlibs
LDFLAGS        += -nolibc
LDFLAGS        += -nostdlib
LDFLAGS        += -Tlink.ld
LDFLAGS        += -Wl,-q
# Disable the new linker warning '--warn-rwx-segments' introduced by
# Binutils 2.39, which causes the following message: "warning:
# $(TARGET).elf has a LOAD segment with RWX permissions".
ifeq ($(shell $(PREFIX)ld --help | grep -q 'warn-rwx-segments'; echo $$?), 0)
LDFLAGS        += -Wl,--no-warn-rwx-segments
endif

OBJCOPYFLAGS    = --input-target=elf32-littlearm
OBJCOPYFLAGS   += --output-target=binary

TARGET          = led

all: $(TARGET).bin gdbinit

$(TARGET).bin: $(TARGET)-raw.bin padding.bin
	cat $^ > $@

$(TARGET)-raw.bin: crt0.bin symbols.bin relocation.bin partition.bin
	cat $^ > $@

crt0.bin: crt0/crt0.c crt0/link.ld crt0/Makefile
	make -C crt0 realclean all
	cp crt0/$@ $@

symbols.bin: $(TARGET).elf scripts/symbols.py
	exec scripts/symbols.py $< $@

relocation.bin: $(TARGET).elf scripts/relocation.py
	exec scripts/relocation.py $< $@

partition.bin: $(TARGET).elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	@chmod 644 $@

$(TARGET).elf: main.o stdriot/stdriot.o
	$(LD) $(LDFLAGS) $^ -o $@

stdriot/stdriot.o: stdriot/stdriot.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

padding.bin: $(TARGET)-raw.bin scripts/padding.py
	exec scripts/padding.py $< $@

gdbinit: scripts/gdbinit.py crt0/crt0.elf $(TARGET).bin crt0.bin symbols.bin relocation.bin
	exec scripts/gdbinit.py\
            $(shell realpath crt0/crt0.elf)\
            $(shell realpath $(TARGET).elf)\
            $$(($$(wc -c < crt0.bin)+$$(wc -c < symbols.bin)+$$(wc -c < relocation.bin))) > $@

clean:
	$(RM)\
            main.o\
            stdriot/stdriot.o\
            $(TARGET)-raw.bin\
            padding.bin\
            crt0.bin\
            symbols.bin\
            relocation.bin\
            partition.bin
	make -C crt0 clean

realclean: clean
	$(RM) $(TARGET).elf $(TARGET).bin gdbinit
	make -C crt0 realclean

.PHONY: all clean realclean
