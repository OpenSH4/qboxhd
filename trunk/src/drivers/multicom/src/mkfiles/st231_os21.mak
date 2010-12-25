#
# mkfiles/st231_os21.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Setup macros for OS21 on ST231
#

ifndef DISABLE_ST231_OS21
  ifneq (,$(call FINDEXEC,st200cc,st200cc.exe,ST200CC.EXE))
    ENABLE_ST231_OS21 = 1
  endif
endif

# Hack to attempt to support the 7111 under the old ST200 compilers
#
# Drop the -msoc argument for all but the st200-r6.x compilers
ifeq (,$(findstring R6,$(call TOUPPER,$(call FINDEXEC,st200cc,st200cc.exe,ST200CC.EXE))))
SOC_ST231_OS21 =
endif

OBJDIR_ST231_OS21   = obj/os21/st231
ifdef ENABLE_PLATFORM_SPECIFIC_OBJECTS
OBJDIR_ST231_OS21  := $(OBJDIR_ST231_OS21)/$(PLATFORM)
endif

ifdef ENABLE_ST231_OS21

CLEAN_DIRS             += $(OBJDIR_ST231_OS21)/*
MAKE_DIRS              += $(OBJDIR_ST231_OS21)
LIBDIR_ST231_OS21       = $(RPC_ROOT)/lib/os21/st231

CC_ST231_OS21        = st200cc
CPP_ST231_OS21       = st200cc -E
CFLAGS_ST231_OS21    = -pedantic -ansi -Wall -mruntime=os21 -mcore=st231 \
                       $(DEFS_ST231_OS21) $(OPT_ST231_OS21) $(DEBUG_ST231_OS21) \
		       $(LOCAL_CFLAGS) $(LOCAL_CFLAGS_ST231_OS21) $(DEBUG_CFLAGS) $(INCS_ST231_OS21)
DEFS_ST231_OS21      = -D__OS21__ -D__ST200__ -D__ST231__
#DEFS_ST231_OS21     += -DEMBX_RECEIVE_CALLBACK
INCS_ST231_OS21      = -I$(RPC_ROOT)/include

ifdef ENABLE_RPCCC
CC_ST231_OS21       := rpccc $(CC_ST231_OS21)
endif
ifdef ENABLE_DEBUG
OPT_ST231_OS21       = -Os
DEBUG_ST231_OS21     = -g
else
OPT_ST231_OS21       = -Os
DEBUG_ST231_OS21     =
endif

OBJ_ST231_OS21       = o

$(OBJDIR_ST231_OS21) :
	-$(MKDIR) $(call DOSCMD,$@)

ifdef TARGETDIR_ST231_OS21
TARGETDIR_LDFLAGS_ST231_OS21=-mtargetdir=$(subst \,/,$(TARGETDIR_ST231_OS21))
TARGETDIR_RTLDFLAGS_ST231_OS21=--mtargetdir=$(subst \,/,$(TARGETDIR_ST231_OS21))
endif

ifndef TARGETDIR_RTLDFLAGS_ST231_OS21
ifdef ST200ROOT_TARGET
TARGETDIR_RTLDFLAGS_ST231_OS21=--mtargetdir=$(subst \,/,$(ST200ROOT_TARGET))
endif
endif

LIB_ST231_OS21       = st200ar
LIBFLAGS_ST231_OS21  = -rcs $(LOCAL_LIBFLAGS_ST231_OS21)

LD_ST231_OS21        = st200cc
LDFLAGS_ST231_OS21   = -mruntime=os21 -mboard=$(BOARD_ST231_OS21) \
                       $(SOC_ST231_OS21) -mcore=st231 \
		       $(TARGETDIR_LDFLAGS_ST231_OS21) \
                      -L$(LIBDIR_ST231_OS21) \
		      -Wl,-Map=$@.map $(LOCAL_LDFLAGS_ST231_OS21) $(LIBS_ST231_OS21)
LIBS_ST231_OS21      = -lembxshell

ifdef ENABLE_STMC2
RTLD_ST231_OS21      = st200xrun -c st200tp -t "$2" $(LOCAL_RTLDFLAGS_ST231_OS21) -e $3
else
RTLD_ST231_OS21      = st200run -t $2 $(LOCAL_RTLDFLAGS_ST231_OS21) $(TARGETDIR_RTLDFLAGS_ST231_OS21) $3
endif

.SUFFIXES: .$(OBJ_ST231_OS21) .c

$(OBJDIR_ST231_OS21)/%.$(OBJ_ST231_OS21) : %.c
ifdef ENABLE_COMENTARY
	@$(ECHO) +++ Compile ST231/OS21 source [$<]. +++
endif
	$(CC_ST231_OS21) -c $(CFLAGS_ST231_OS21) $< -o $@ 

$(OBJDIR_ST231_OS21)/%_core1.$(OBJ_ST231_OS21) : %.c
ifdef ENABLE_COMENTARY
	@$(ECHO) +++ Compile ST231/OS21 source [$<]. +++
endif
	$(CC_ST231_OS21) -c $(CFLAGS_ST231_OS21) $< -o $@ 

ifdef OBJS_ST231_OS21

ifdef LIBRARY
TARGET_FOR_ST231_OS21 = $(OBJDIR_ST231_OS21)/lib$(LIBRARY).a
DIST_ST231_OS21_LIB  += $(TARGET_FOR_ST231_OS21)

$(TARGET_FOR_ST231_OS21) : $(OBJS_ST231_OS21)
	$(LIB_ST231_OS21) $(LIBFLAGS_ST231_OS21) $@ $(OBJS_ST231_OS21)
endif

ifdef APPLICATION
TARGET_FOR_ST231_OS21 = $(OBJDIR_ST231_OS21)/$(APPLICATION).out
DIST_ST231_OS21_LIB  += $(TARGET_FOR_ST231_OS21)

$(TARGET_FOR_ST231_OS21) : $(OBJS_ST231_OS21)
	$(LD_ST231_OS21) -o $@ $(OBJS_ST231_OS21) $(LDFLAGS_ST231_OS21)
endif 

endif # OBJS_ST231_OS21

MAKE_TARGETS         += $(TARGET_FOR_ST231_OS21)

endif # ENABLE_ST231_OS21

