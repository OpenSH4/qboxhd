#
# mkfiles/ia32_winnt.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Setup macros for SH4 WinNT userspace
#

ifndef DISABLE_IA32_WINNT
  ifneq (,$(call FINDEXEC,cl.exe,CL.EXE))
    ENABLE_IA32_WINNT = 1
  endif
endif

OBJDIR_IA32_WINNT   = obj/winnt/ia32
ifdef ENABLE_PLATFORM_SPECIFIC_OBJECTS
OBJDIR_IA32_WINNT  := $(OBJDIR_IA32_WINNT)/$(PLATFORM)
endif

ifdef ENABLE_IA32_WINNT

CLEAN_DIRS             += $(OBJDIR_IA32_WINNT)/*
MAKE_DIRS              += $(OBJDIR_IA32_WINNT)
LIBDIR_IA32_WINNT       = $(RPC_ROOT)/lib/winnt/ia32

CC_IA32_WINNT        = cl
CPP_IA32_WINNT       = cl -E
# /nologo - do not issue any splash information
# /W3     - set the warning level to 3
CFLAGS_IA32_WINNT    = -nologo -W3 \
                       $(DEFS_IA32_WINNT) $(OPT_IA32_WINNT) $(DEBUG_IA32_WINNT) \
		       $(LOCAL_CFLAGS) $(LOCAL_CFLAGS_IA32_WINNT) $(DEBUG_CFLAGS) $(INCS_IA32_WINNT)
DEFS_IA32_WINNT      = -D__WINNT__ -D__IA32__ -DWIN32
INCS_IA32_WINNT      = -I"$(RPC_ROOT)/include"

ifdef ENABLE_RPCCC
CC_IA32_WINNT       := rpccc $(CC_IA32_WINNT)
endif
ifdef ENABLE_DEBUG
OPT_IA32_WINNT       = 
DEBUG_IA32_WINNT     = -Yd
else
OPT_IA32_WINNT       = -O2
DEBUG_IA32_WINNT     =
endif

OBJ_IA32_WINNT       = obj

$(OBJDIR_IA32_WINNT) :
	-$(MKDIR) $(call DOSCMD,$@)

LIB_IA32_WINNT       = link
LIBFLAGS_IA32_WINNT  = -nologo -dll -incremental:no $(LOCAL_LIBFLAGS_IA32_WINNT)

LD_IA32_WINNT        = cl
LDFLAGS_IA32_WINNT   = $(LOCAL_LDFLAGS_IA32_WINNT)

RTLD_IA32_WINNT      = 

.SUFFIXES: .$(OBJ_IA32_WINNT) .c

$(OBJDIR_IA32_WINNT)/%.$(OBJ_IA32_WINNT) : %.c
ifdef ENABLE_COMENTARY
	@$(ECHO) +++ Compile IA32/WinNT source [$<]. +++
endif
	$(CC_IA32_WINNT) -c $(CFLAGS_IA32_WINNT) $< -Fo$@ 

ifdef OBJS_IA32_WINNT

ifdef LIBRARY
TARGET_FOR_IA32_WINNT = $(OBJDIR_IA32_WINNT)/$(LIBRARY).dll
DIST_IA32_WINNT_LIB  += $(TARGET_FOR_IA32_WINNT)

$(TARGET_FOR_IA32_WINNT) : $(OBJS_IA32_WINNT)
	$(LIB_IA32_WINNT) $(LIBFLAGS_IA32_WINNT) /out:$@ $(OBJS_IA32_WINNT)
endif

ifdef APPLICATION
TARGET_FOR_IA32_WINNT = $(OBJDIR_IA32_WINNT)/$(APPLICATION).exe
DIST_IA32_WINNT_BIN  += $(TARGET_FOR_IA32_WINNT)

$(TARGET_FOR_IA32_WINNT) : $(OBJS_IA32_WINNT)
	$(LD_IA32_WINNT) -o $@ $(OBJS_IA32_WINNT) $(LDFLAGS_IA32_WINNT)
endif 

endif # OBJS_IA32_WINNT

MAKE_TARGETS         += $(TARGET_FOR_IA32_WINNT)

endif # ENABLE_IA32_WINNT

