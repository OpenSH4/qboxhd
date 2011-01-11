#
# Makefile --
#
# Description: Makefile for building the shared library that provides
#              basic functionality to higher-level apps
#
# Copyright (c) 2007 Duolabs S.r.l.
#
# Changelog:
# Date    Author      Comments
# ----------------------------------------------------------------------------
# 301007  paguilar    Original
#

CC = sh4-linux-gcc
STRIP = sh4-linux-strip

TARGET = libduoutils.so
V1 = 1
V2 = 0
V3 = 2

# FIXME: 
# -fpic Result code is smaller and faster, but not sure if it works on SH.
# -fomit-frame-pointer avoids the use of the frame pointer making the debugger useless.
#CFLAGS = -O2 -Wall -fpic -fomit-frame-pointer
CFLAGS = -O2 -Wall -fPIC

# -export-dynamic could be use if the lib uses unresolved external symbols
# that must be defined by the program that uses it
#LDFLAGS = -shared -Wl,-export-dynamic -Wl,-soname
LDFLAGS = -shared -Wl,-soname

DUOCOMM_SRCS += *.c
DUOCOMM_OBJS += $(patsubst %.c, %.o, $(DUOCOMM_SRCS))

all: clean
	@echo Compiling sources...
	$(CC) $(CFLAGS) -c $(DUOCOMM_SRCS)
	@echo Generating shared library...
	@$(CC) -shared -Wl,-soname,$(TARGET) -Wl,-soname,$(TARGET).$(V1) \
	-o $(TARGET).$(V1).$(V2).$(V3) $(DUOCOMM_OBJS) -lc
	@echo Stripping library
	@$(STRIP) $(TARGET).$(V1).$(V2).$(V3)
	@echo Creating logical links...
	@ln -s $(TARGET).$(V1).$(V2).$(V3) $(TARGET) 
	@ln -s $(TARGET).$(V1).$(V2).$(V3) $(TARGET).$(V1)

objclean:
	@echo Removing object files...
	@rm -f *.o

clean:
	@echo Cleaning...
	@rm -f $(TARGET)* *.o


