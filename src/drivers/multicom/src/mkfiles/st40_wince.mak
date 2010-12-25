#
# mkfiles/st40_wince.mak
#
# Copyright (C) STMicroelectronics Ltd. 2005
#
# Setup macros for ST40 WinCE
#

OBJDIR_ST40_WINCE   = obj/wince/st40
ifdef ENABLE_PLATFORM_SPECIFIC_OBJECTS
OBJDIR_ST40_WINCE  := $(OBJDIR_ST40_WINCE)/$(PLATFORM)
endif

ifdef ENABLE_ST40_WINCE

CLEAN_DIRS             += $(OBJDIR_ST40_WINCE)/*
MAKE_DIRS              += $(OBJDIR_ST40_WINCE)
LIBDIR_ST40_WINCE       = $(RPC_ROOT)/lib/wince/st40

CC_ST40_WINCE        = cl
CPP_ST40_WINCE       = $(CC_ST40_WINCE) -E

# /nologo - do not issue any splash information
# /W3     - set the warning level to 3
CFLAGS_ST40_WINCE    = -nologo -W3 \
                       $(DEFS_ST40_WINCE) \
		       $(OPT_ST40_WINCE) \
		       $(DEBUG_ST40_WINCE) \
		       $(LOCAL_CFLAGS) \
		       $(LOCAL_CFLAGS_ST40_WINCE) \
		       $(DEBUG_CFLAGS) \
		       $(INCS_ST40_WINCE)
DEFS_ST40_WINCE      = -D__WINCE__ -D__SH4__ -DSHx -DSH4 -DUNDER_CE -DUNICODE -D_UNICODE
#DEFS_ST40_WINCE     += -DEMBX_RECEIVE_CALLBACK
INCS_ST40_WINCE      = -I$(RPC_ROOT)/include \
		       -I$(_TARGETPLATROOT)/inc \
		       -I$(_TARGETPLATROOT)/src/inc \
		       -I$(_PLATFORMROOT)/common/src/inc \
		       -I$(_WINCEROOT)\public\common\ddk\inc \
		       -I$(_WINCEROOT)\public\common\oak\inc \
		       -I$(_WINCEROOT)\public\common\sdk\inc \
		       -I$(_WINCEROOT)\PUBLIC\COMMON\OAK\CSP\SHX\INC \
		       -I$(_WINCEROOT)\PUBLIC\COMMON\OAK\CSP\SHX\SH4\INC \
		       -I$(_WINCEROOT)\PUBLIC\COMMON\OAK\CSP\SHX\SH4\StMicro\SH4_202T\Inc



ifdef ENABLE_RPCCC
CC_ST40_WINCE       := rpccc $(CC_ST40_WINCE)
endif
ifdef ENABLE_DEBUG
OPT_ST40_WINCE       = 
DEBUG_ST40_WINCE     = -Z7 -Yd
else
OPT_ST40_WINCE       = -O2
DEBUG_ST40_WINCE     =
endif

OBJ_ST40_WINCE       = obj

$(OBJDIR_ST40_WINCE) :
	-$(MKDIR) $(call DOSCMD,$@)

LIB_ST40_WINCE       = link
# a command line parsing bug in link.exe means that -lib must appear on the
# link line before anything else
LIBFLAGS_ST40_WINCE  = -lib -nologo -subsystem:WINDOWSCE

LD_ST40_WINCE        = link
# -entry:mainACRTStartup ensures we enter our program using the ANSI C entry
# point (main) rather than the Windows entry point (WinMain)
LDFLAGS_ST40_WINCE   = -nologo \
		       -machine:SH4 \
		       -subsystem:WINDOWSCE -entry:mainACRTStartup \
		       $(LOCAL_LDFLAGS_ST40_WINCE) $(LIBS_ST40_WINCE) \
		       -libpath:$(LIBDIR_ST40_WINCE) \
		       -libpath:$(_PROJECTROOT)/cesysgen/sdk/lib/$(_TGTCPU)/$(WINCEDEBUG) \
		       -libpath:$(_WINCEROOT)/public/cebase/cesysgen/sdk/lib/$(_TGTCPU)/$(WINCEDEBUG)

LIBS_ST40_WINCE      = embxshell.lib

RTLD_ST40_WINCE      = 

.SUFFIXES: .$(OBJ_ST40_WINCE) .c

$(OBJDIR_ST40_WINCE)/%.$(OBJ_ST40_WINCE) : %.c
ifdef ENABLE_COMENTARY
	@$(ECHO) +++ Compile ST40/WinCE source [$<]. +++
endif
	$(CC_ST40_WINCE) -c $(CFLAGS_ST40_WINCE) $< -Fo$@ 

ifdef OBJS_ST40_WINCE

ifdef LIBRARY
TARGET_FOR_ST40_WINCE = $(OBJDIR_ST40_WINCE)/$(LIBRARY).lib
DIST_ST40_WINCE_LIB  += $(TARGET_FOR_ST40_WINCE)

$(TARGET_FOR_ST40_WINCE) : $(OBJS_ST40_WINCE)
	$(LIB_ST40_WINCE) $(LIBFLAGS_ST40_WINCE) -out:$@ $(OBJS_ST40_WINCE)
endif

ifdef APPLICATION
TARGET_FOR_ST40_WINCE = $(OBJDIR_ST40_WINCE)/$(APPLICATION).exe
DIST_ST40_WINCE_BIN  += $(TARGET_FOR_ST40_WINCE)

$(TARGET_FOR_ST40_WINCE) : $(OBJS_ST40_WINCE)
	$(LD_ST40_WINCE) -out:$@ $(OBJS_ST40_WINCE) $(LDFLAGS_ST40_WINCE)
endif 

endif # OBJS_ST40_WINCE

MAKE_TARGETS         += $(TARGET_FOR_ST40_WINCE)

endif # ENABLE_ST40_WINCE

