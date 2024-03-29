#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

ifeq ($(ARCH),ppc)
LOAD_ADDR = 0x40000
endif

ifeq ($(ARCH),i386)
LOAD_ADDR = 0x40000
endif

ifeq ($(ARCH),arm)
LOAD_ADDR = 0xc100000
endif

ifeq ($(ARCH),mips)
LOAD_ADDR = 0x80200000 -T mips.lds
endif

ifeq ($(ARCH),nios)
LOAD_ADDR = 0x00800000 -L $(gcclibdir)/m32 -T nios.lds
endif

ifeq ($(ARCH),nios2)
LOAD_ADDR = 0x00800000 -L $(gcclibdir) -T nios2.lds
endif

ifeq ($(ARCH),m68k)
LOAD_ADDR = 0x20000  -L $(clibdir)
endif

ifeq ($(ARCH),microblaze)
LOAD_ADDR = 0x80F00000
endif

ifeq ($(ARCH),sh4)
include $(TOPDIR)/include/config.mk
ifneq (,$(findstring $(BOARD),espresso stb7100mboard stb7100ref))
LOAD_ADDR = 0x84000000
else
LOAD_ADDR = 0x88000000
endif
endif

include $(TOPDIR)/config.mk

ELF 	= hello_world

ifeq ($(ARCH),i386)
ELF    += 82559_eeprom
endif

ifeq ($(ARCH),ppc)
SREC   += sched.srec
BIN    += sched.bin sched
endif

# The following example is pretty 8xx specific...
ifeq ($(CPU),mpc8xx)
ELF    += timer
endif

# The following example is 8260 specific...
ifeq ($(CPU),mpc8260)
ELF    += mem_to_mem_idma2intr
endif

# Utility for resetting i82559 EEPROM
ifeq ($(BOARD),oxc)
ELF   += eepro100_eeprom
endif

ifeq ($(BIG_ENDIAN),y)
EX_LDFLAGS += -EB
endif

SREC  = $(addsuffix .srec,$(ELF))
BIN   = $(addsuffix .bin,$(ELF))
OBJS	= $(SREC:.srec=.o)

LIB	= libstubs.a
LIBAOBJS=
ifeq ($(ARCH),ppc)
LIBAOBJS+= $(ARCH)_longjmp.o $(ARCH)_setjmp.o
endif
LIBCOBJS= stubs.o
LIBOBJS	= $(LIBAOBJS) $(LIBCOBJS)

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)
clibdir := $(shell dirname `$(CC) $(CFLAGS) -print-file-name=libc.a`)

CPPFLAGS += -I..

all:	.depend $(LIB) $(SREC) $(BIN)

#########################################################################
$(LIB): .depend $(LIBOBJS)
	$(AR) crv $@ $(LIBOBJS)

$(ELF): %: %.o $(LIB)
	$(LD) -g $(EX_LDFLAGS) -Ttext $(LOAD_ADDR) \
		-o $@ -e $(<:.o=) $< $(LIB) \
		-L$(gcclibdir) -lgcc

$(SREC): %.srec: %
	$(OBJCOPY) -O srec $< $@ 2>/dev/null

$(BIN): %.bin: %
	$(OBJCOPY) -O binary $< $@ 2>/dev/null

#########################################################################

.depend:	Makefile $(OBJS:.o=.c) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) $(LIBCOBJS:.o=.c) $(LIBAOBJS:.o=.S) > $@

sinclude .depend

#########################################################################
