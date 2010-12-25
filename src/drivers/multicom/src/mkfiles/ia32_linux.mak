#
# mkfiles/ia32_linux.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Setup macros for SH4 Linux userspace
#

ifndef DISABLE_IA32_LINUX
  ifneq (,$(call FINDEXEC,gcc))
    # gcc is found on most Unix boxen so we'd better check the host type as well
    ifeq ($(HOSTTYPE),Linux)
      ENABLE_IA32_LINUX = 1
    endif
  endif
endif

OBJDIR_IA32_LINUX   = obj/linux/ia32
ifdef ENABLE_PLATFORM_SPECIFIC_OBJECTS
OBJDIR_IA32_LINUX  := $(OBJDIR_IA32_LINUX)/$(PLATFORM)
endif

ifdef ENABLE_IA32_LINUX

CLEAN_DIRS             += $(OBJDIR_IA32_LINUX)/*
MAKE_DIRS              += $(OBJDIR_IA32_LINUX)
LIBDIR_IA32_LINUX       = $(RPC_ROOT)/lib/linux/ia32

CC_IA32_LINUX        = gcc
CPP_IA32_LINUX       = gcc -E
CFLAGS_IA32_LINUX    = -pedantic -ansi -Wall \
                       $(DEFS_IA32_LINUX) $(OPT_IA32_LINUX) $(DEBUG_IA32_LINUX) \
		       $(LOCAL_CFLAGS) $(LOCAL_CFLAGS_IA32_LINUX) $(DEBUG_CFLAGS) $(INCS_IA32_LINUX)
DEFS_IA32_LINUX      = -D__LINUX__ -D__IA32__
INCS_IA32_LINUX      = -I$(RPC_ROOT)/include

ifdef ENABLE_RPCCC
CC_IA32_LINUX       := rpccc $(CC_IA32_LINUX)
endif
ifdef ENABLE_DEBUG
OPT_IA32_LINUX       = 
DEBUG_IA32_LINUX     = -g
else
OPT_IA32_LINUX       = -O3
DEBUG_IA32_LINUX     =
endif

OBJ_IA32_LINUX       = o

$(OBJDIR_IA32_LINUX) :
	-$(MKDIR) $(call DOSCMD,$@)

LIB_IA32_LINUX       = ar
LIBFLAGS_IA32_LINUX  = -rcs $(LOCAL_LIBFLAGS_IA32_LINUX)

LD_IA32_LINUX        = gcc
LDFLAGS_IA32_LINUX   = $(LOCAL_LDFLAGS_IA32_LINUX)

RTLD_IA32_LINUX      = /usr/bin/env

.SUFFIXES: .$(OBJ_IA32_LINUX) .c

$(OBJDIR_IA32_LINUX)/%.$(OBJ_IA32_LINUX) : %.c
ifdef ENABLE_COMENTARY
	@$(ECHO) +++ Compile IA32/Linux source [$<]. +++
endif
	$(CC_IA32_LINUX) -c $(CFLAGS_IA32_LINUX) $< -o $@ 

ifdef OBJS_IA32_LINUX

ifdef LIBRARY
TARGET_FOR_IA32_LINUX = $(OBJDIR_IA32_LINUX)/lib$(LIBRARY).a
DIST_IA32_LINUX_LIB  += $(TARGET_FOR_IA32_LINUX)

$(TARGET_FOR_IA32_LINUX) : $(OBJS_IA32_LINUX)
	$(LIB_IA32_LINUX) $(LIBFLAGS_IA32_LINUX) $@ $(OBJS_IA32_LINUX)
endif

ifdef APPLICATION
TARGET_FOR_IA32_LINUX = $(OBJDIR_IA32_LINUX)/$(APPLICATION)
DIST_IA32_LINUX_BIN  += $(TARGET_FOR_IA32_LINUX)

$(TARGET_FOR_IA32_LINUX) : $(OBJS_IA32_LINUX)
	$(LD_IA32_LINUX) -o $@ $(OBJS_IA32_LINUX) $(LDFLAGS_IA32_LINUX)
endif 

endif # OBJS_IA32_LINUX

MAKE_TARGETS         += $(TARGET_FOR_IA32_LINUX)

endif # ENABLE_IA32_LINUX

