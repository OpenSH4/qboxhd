#
# mkfiles/st40_linux.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Setup macros for SH4 Linux userspace
#

ifndef DISABLE_ST40_LINUX
  ifneq (,$(call FINDEXEC,sh4-linux-gcc))
    ENABLE_ST40_LINUX = 1
  endif
endif

OBJDIR_ST40_LINUX   = obj/linux/st40
ifdef ENABLE_PLATFORM_SPECIFIC_OBJECTS
OBJDIR_ST40_LINUX  := $(OBJDIR_ST40_LINUX)/$(PLATFORM)
endif

ifdef ENABLE_ST40_LINUX
DIST_ST40_LINUX_GCC=$(call FINDEXEC,sh4-linux-gcc)

# switch automatically between versions of the distribution
DIST_ST40_LINUX_1_0 = /opt/STM/ST40Linux-1.0
DIST_ST40_LINUX_2_0 = /opt/STM/STLinux-2.0
DIST_ST40_LINUX_2_2 = /opt/STM/STLinux-2.2
DIST_ST40_LINUX_2_3 = /opt/STM/STLinux-2.3
ifeq ($(DIST_ST40_LINUX_1_0),$(findstring $(DIST_ST40_LINUX_1_0),$(DIST_ST40_LINUX_GCC)))
DIST_ST40_LINUX ?= $(DIST_ST40_LINUX_1_0)
else
ifeq ($(DIST_ST40_LINUX_2_0),$(findstring $(DIST_ST40_LINUX_2_0),$(DIST_ST40_LINUX_GCC)))
DIST_ST40_LINUX ?= $(DIST_ST40_LINUX_2_0)
else
ifeq ($(DIST_ST40_LINUX_2_2),$(findstring $(DIST_ST40_LINUX_2_2),$(DIST_ST40_LINUX_GCC)))
DIST_ST40_LINUX ?= $(DIST_ST40_LINUX_2_2)
else
DIST_ST40_LINUX ?= $(DIST_ST40_LINUX_2_3)
endif
endif
endif

CLEAN_DIRS             += $(OBJDIR_ST40_LINUX)/*
MAKE_DIRS              += $(OBJDIR_ST40_LINUX)
LIBDIR_ST40_LINUX       = $(RPC_ROOT)/lib/linux/st40

CC_ST40_LINUX        = sh4-linux-gcc
CPP_ST40_LINUX       = sh4-linux-gcc -E
CFLAGS_ST40_LINUX    = -Wall -fpic $(DEFS_ST40_LINUX) $(OPT_ST40_LINUX) $(DEBUG_ST40_LINUX) \
		       $(LOCAL_CFLAGS) $(LOCAL_CFLAGS_ST40_LINUX) $(DEBUG_CFLAGS) $(INCS_ST40_LINUX)
DEFS_ST40_LINUX      = -D__LINUX__ -D__SH4__
INCS_ST40_LINUX      = -I$(RPC_ROOT)/include

ifdef ENABLE_RPCCC
CC_ST40_LINUX       := rpccc $(CC_ST40_LINUX)
endif
ifdef ENABLE_DEBUG
OPT_ST40_LINUX       = -O2 -fomit-frame-pointer -fno-strict-aliasing
DEBUG_ST40_LINUX     = -g
else
OPT_ST40_LINUX       = -O2 -fomit-frame-pointer -fno-strict-aliasing
DEBUG_ST40_LINUX     = -g
endif

OBJ_ST40_LINUX       = o

ifndef OBJDIR_ST40_LINUX_MKDIR_RULE_ALREADY_DONE
OBJDIR_ST40_LINUX_MKDIR_RULE_ALREADY_DONE = 1
$(OBJDIR_ST40_LINUX) :
	-$(MKDIR) $(call DOSCMD,$@)
endif

LIB_ST40_LINUX       = sh4-linux-ld
LIBFLAGS_ST40_LINUX  = -shared -EL $(LOCAL_LIBFLAGS_ST40_LINUX) -o

LD_ST40_LINUX        = sh4-linux-gcc
LDFLAGS_ST40_LINUX   = -L$(LIBDIR_ST40_LINUX) $(LOCAL_LDFLAGS_ST40_LINUX)

# TODO: the use of global macros in this function will prevent us from testing
#       multiple Linux on a single SoC.
#
ifdef ENABLE_STMC2
RTLD_ST40_LINUX       = env RPC_ROOT=$(RPC_ROOT) \
			st40load_gdb -c sh4tp -t $2 -b $(RTLD_KERNEL_ST40_LINUX)\
		       -z $(RTLD_RAMDISKADDR_ST40_LINUX),$(OBJDIR_ST40_LINUX)/$(TEST).img\
		       -- $(RTLD_COMMANDLINE_ST40_LINUX)
else
RTLD_ST40_LINUX      = env RPC_ROOT=$(RPC_ROOT) \
			st40load_gdb -c $1 -t $2 -b $(RTLD_KERNEL_ST40_LINUX)\
		       -z $(RTLD_RAMDISKADDR_ST40_LINUX),$(OBJDIR_ST40_LINUX)/$(TEST).img\
		       -- $(RTLD_COMMANDLINE_ST40_LINUX)
endif

ifndef RTLD_KERNEL_ST40_LINUX
RTLD_KERNEL_ST40_LINUX = $(error RTLD_KERNEL_ST40_LINUX is not set)
endif
ifndef RTLD_RAMDISKADDR_ST40_LINUX
RTLD_RAMDISKADDR_ST40_LINUX = $(error RTLD_RAMDISKADDR_ST40_LINUX is not set)
endif
ifndef RTLD_COMMANDLINE_ST40_LINUX
RTLD_COMMANDLINE_ST40_LINUX = $(error RTLD_COMMANDLINE_ST40_LINUX is not set)
endif

.SUFFIXES: .$(OBJ_ST40_LINUX) .c

$(OBJDIR_ST40_LINUX)/%.$(OBJ_ST40_LINUX) : %.c
ifdef ENABLE_COMENTARY
	@$(ECHO) +++ Compile ST40/Linux source [$<]. +++
endif
	$(CC_ST40_LINUX) -c $(CFLAGS_ST40_LINUX) $< -o $@ 

ifdef OBJS_ST40_LINUX

ifdef LIBRARY
TARGET_FOR_ST40_LINUX = $(OBJDIR_ST40_LINUX)/lib$(LIBRARY).so
DIST_ST40_LINUX_LIB  += $(TARGET_FOR_ST40_LINUX)

$(TARGET_FOR_ST40_LINUX) : $(OBJS_ST40_LINUX)
	$(LIB_ST40_LINUX) $(LIBFLAGS_ST40_LINUX) $@ $(OBJS_ST40_LINUX)
endif

ifdef APPLICATION
TARGET_FOR_ST40_LINUX = $(OBJDIR_ST40_LINUX)/$(APPLICATION).out
DIST_ST40_LINUX_BIN  += $(TARGET_FOR_ST40_LINUX)

$(TARGET_FOR_ST40_LINUX) : $(OBJS_ST40_LINUX)
	$(LD_ST40_LINUX) -o $@ $(OBJS_ST40_LINUX) $(LDFLAGS_ST40_LINUX)
endif 

endif # OBJS_ST40_LINUX

MAKE_TARGETS         += $(TARGET_FOR_ST40_LINUX)

endif # ENABLE_ST40_LINUX

