#
# mkfiles/st40_os21.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Setup macros for OS21 on SH4 
#

ifndef DISABLE_ST40_OS21
  ifneq (,$(call FINDEXEC,sh4gcc,sh4gcc.exe,SH4GCC.EXE))
    ENABLE_ST40_OS21 = 1
  endif
endif

OBJDIR_ST40_OS21   = obj/os21/st40
ifdef ENABLE_PLATFORM_SPECIFIC_OBJECTS
OBJDIR_ST40_OS21  := $(OBJDIR_ST40_OS21)/$(PLATFORM)
endif

ifdef ENABLE_ST40_OS21

CLEAN_DIRS             += $(OBJDIR_ST40_OS21)/*
MAKE_DIRS              += $(OBJDIR_ST40_OS21)
LIBDIR_ST40_OS21       = $(RPC_ROOT)/lib/os21/st40

CC_ST40_OS21        = sh4gcc
CPP_ST40_OS21       = sh4gcc -E
CFLAGS_ST40_OS21    = -pedantic -ansi -Wall -mruntime=os21 \
                      $(DEFS_ST40_OS21) $(OPT_ST40_OS21) $(DEBUG_ST40_OS21) \
		      $(LOCAL_CFLAGS) $(LOCAL_CFLAGS_ST40_OS21) $(DEBUG_CFLAGS) $(INCS_ST40_OS21)
DEFS_ST40_OS21      = -D__OS21__ -D__SH4__
#DEFS_ST40_OS21     += -DEMBX_RECEIVE_CALLBACK
INCS_ST40_OS21      = -I$(RPC_ROOT)/include

ifdef ENABLE_RPCCC
CC_ST40_OS21       := rpccc $(CC_ST40_OS21)
endif
ifdef ENABLE_DEBUG
OPT_ST40_OS21       = -Os -fno-strict-aliasing
DEBUG_ST40_OS21     = -g
else
OPT_ST40_OS21       = -Os -fno-strict-aliasing
DEBUG_ST40_OS21     = -g
endif

OBJ_ST40_OS21       = o

$(OBJDIR_ST40_OS21) :
	-$(MKDIR) $(call DOSCMD,$@)

LIB_ST40_OS21       = sh4ar
LIBFLAGS_ST40_OS21  = -rcs $(LOCAL_LIBFLAGS_ST40_OS21)

LD_ST40_OS21        = sh4gcc
LDFLAGS_ST40_OS21   = -mruntime=os21 -mboard=$(BOARD_ST40_OS21)$(REGION_ST40_OS21) \
                      -L$(LIBDIR_ST40_OS21) \
		      -Xlinker -Map=$@.map $(LOCAL_LDFLAGS_ST40_OS21) $(LIBS_ST40_OS21)
LIBS_ST40_OS21      = -lembxshell
#REGION_ST40_OS21    = p1

ifdef ENABLE_STMC2
RTLD_ST40_OS21      = sh4xrun -c sh4tp -t $2 $(LOCAL_RTLDFLAGS_ST40_OS21) -e $3
else
RTLD_ST40_OS21      = sh4xrun -i $(RPC_ROOT)/configs/st40defs.cmd -c $1 -t $2 $(LOCAL_RTLDFLAGS_ST40_OS21) -e $3
endif

ifndef RTLD_TARGET_ST40_OS21
RTLD_TARGET_ST40_OS21 = $(error RTLD_TARGET_ST40_OS21 is not set)
endif

.SUFFIXES: .$(OBJ_ST40_OS21) .c

$(OBJDIR_ST40_OS21)/%.$(OBJ_ST40_OS21) : %.c
ifdef ENABLE_COMENTARY
	@$(ECHO) +++ Compile ST40/OS21 source [$<]. +++
endif
	$(CC_ST40_OS21) -c $(CFLAGS_ST40_OS21) $< -o $@ 

ifdef OBJS_ST40_OS21

ifdef LIBRARY
TARGET_FOR_ST40_OS21 = $(OBJDIR_ST40_OS21)/lib$(LIBRARY).a
DIST_ST40_OS21_LIB   += $(TARGET_FOR_ST40_OS21)

$(TARGET_FOR_ST40_OS21) : $(OBJS_ST40_OS21)
	$(LIB_ST40_OS21) $(LIBFLAGS_ST40_OS21) $@ $(OBJS_ST40_OS21)
endif

ifdef APPLICATION
TARGET_FOR_ST40_OS21 = $(OBJDIR_ST40_OS21)/$(APPLICATION).out
DIST_ST40_OS21_BIN   += $(TARGET_FOR_ST40_OS21)

$(TARGET_FOR_ST40_OS21) : $(OBJS_ST40_OS21)
	$(LD_ST40_OS21) -o $@ $(OBJS_ST40_OS21) $(LDFLAGS_ST40_OS21)
endif 

endif # OBJS_ST40_OS21

MAKE_TARGETS         += $(TARGET_FOR_ST40_OS21)

endif # ENABLE_ST40_OS21

