#
# mkfiles/platform.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Platform specific defines
#

ENABLE_PLATFORM_SPECIFIC_OBJECTS=1

# fully disable automatic builds
DISABLE_IA32_LINUX=1
DISABLE_IA32_WINNT=1
DISABLE_ST40_LINUX=1
DISABLE_ST40_LINUX_KO=1
DISABLE_ST40_OS21=1
DISABLE_ST40_WINCE=1
DISABLE_ST231_OS21=1
DISABLE_SPARC_SOLARIS=1

# automatic warnings if undefined variables are used
BOARD_0 = $(error BOARD_0 is not defined)
ifndef OS_0
OS_0    = $(error OS_0 is not defined)
endif
ifndef HTI_0
HTI_0   = $(error HTI_0 is not defined)
endif
BOARD_1 = $(error BOARD_1 is not defined)
ifndef OS_1
OS_1    = $(error OS_1 is not defined)
endif
ifndef HTI_1
HTI_1   = $(error HTI_1 is not defined)
endif

ifeq ($(PLATFORM),mb411)
LOCAL_CFLAGS  += -DBOARD_MB411
CORE_0         = st40
BOARD_0        = mb411
CORE_1         = st231
BOARD_1        = mb411_audio
endif # mb411

ifeq ($(PLATFORM),mb442)
LOCAL_CFLAGS  += -DBOARD_MB411
CORE_0         = st40
BOARD_0        = mb442stb7109
CORE_1         = st231
BOARD_1        = mb442_audio
SOC_ST231_OS21 = -msoc=stb7109
endif # mb442

ifeq ($(PLATFORM),mb442se)
LOCAL_CFLAGS  += -DBOARD_MB411 -DMULTICOM_32BIT_SUPPORT
CORE_0         = st40
BOARD_0        = mb442stb7109se
CORE_1         = st231
BOARD_1        = mb442_7109_se_audio
SOC_ST231_OS21 = -msoc=stb7109
endif # mb442se

ifeq ($(PLATFORM),mb519)
LOCAL_CFLAGS  += -DBOARD_MB519
CORE_0         = st40
BOARD_0        = mb519
CORE_1         = st231
BOARD_1        = mb519_audio0
SOC_ST231_OS21 = -msoc=sti7200
endif # mb519

ifeq ($(PLATFORM),mb519se)
LOCAL_CFLAGS  += -DBOARD_MB519 -DMULTICOM_32BIT_SUPPORT
CORE_0         = st40
BOARD_0        = mb519se
CORE_1         = st231
BOARD_1        = mb519_se_audio0
SOC_ST231_OS21 = -msoc=sti7200
endif # mb519se

ifeq ($(PLATFORM),mb618)
LOCAL_CFLAGS  += -DBOARD_MB618
CORE_0         = st40
BOARD_0        = mb618p1
CORE_1         = st231
BOARD_1        = mb618_audio
SOC_ST231_OS21 = -msoc=sti7111
endif # mb618

ifeq ($(PLATFORM),mb618se)
LOCAL_CFLAGS  += -DBOARD_MB618 -DMULTICOM_32BIT_SUPPORT
CORE_0         = st40
BOARD_0        = mb618se
CORE_1         = st231
BOARD_1        = mb618_se_audio
SOC_ST231_OS21 = -msoc=sti7111
endif # mb618se

# if platform is unset then make sure it is an error to expand any of
# the platform macros (the error will only reported during expansion
# so this is quite safe). This means you can still clean/install a
# directory without PLATFORM set, but not build anything.

platforms :
	@$(ECHO) "Supported platforms:"
	@$(ECHO) "    mb411 (st40/st231)"
	@$(ECHO) "    mb442 (st40/st231)"
	@$(ECHO) "    mb442se (st40/st231) (32-bit)"
	@$(ECHO) "    mb519 (st40/st231)"
	@$(ECHO) "    mb519se (st40/st231) (32-bit)"
	@$(ECHO) "    mb618 (st40/st231)"
	@$(ECHO) "    mb618se (st40/st231) (32-bit)"

ifndef PLATFORM
ifndef DISABLE_PLATFORM_WARNINGS
$(warning PLATFORM is not set. This directory will not be built.)
endif
else
ifndef CORE_0
$(error PLATFORM is not set correctly. Type '$(MAKE) platforms' for a list of supported platforms.)
else

#
# manage build macros etc. for the first CPU
#

ifeq ($(CORE_0),st40)
ifeq ($(OS_0),linux)
ENABLE_ST40_LINUX    = 1
BOARD_ST40_LINUX     = $(BOARD)
OBJS_0               = $(foreach f,$(1:.c=.$(OBJ_ST40_LINUX)),$(OBJDIR_ST40_LINUX)/$f)
CPP_0                = $(CPP_ST40_LINUX) $(CPPFLAGS_ST40_LINUX) $2 > $1
APPLICATION_0        = $(OBJDIR_ST40_LINUX)/$1.out
BUILD_APP_0          = $(LD_ST40_LINUX) -o $1 $2 $(LDFLAGS_ST40_LINUX)
LIBRARY_0            = $(OBJDIR_ST40_LINUX)/lib$1.so
BUILD_LIB_0          = $(LIB_ST40_LINUX) $(LIBFLAGS_ST40_LINUX) $1 $2
RTLD_0               = $(call RTLD_ST40_LINUX,$1,$2,$3)
endif # linux
ifeq ($(OS_0),linux_ko)
ENABLE_ST40_LINUX_KO = 1
BOARD_ST40_LINUX_KO  = $(BOARD)
OBJS_0               = $(foreach f,$(1:.c=.$(OBJ_ST40_LINUX_KO)),$(OBJDIR_ST40_LINUX_KO)/$f)
CPP_0                = $(CPP_ST40_LINUX_KO) $(CPPFLAGS_ST40_LINUX_KO) $2 > $1
APPLICATION_0        = $(OBJDIR_ST40_LINUX_KO)/$1.ko
# we haven't included st40_linux_ko.mak at this point so we can't do the 'proper' check
ifneq (,$(findstring ST40Linux-1.0,$(KERNELDIR_ST40_LINUX_KO)))
BUILD_APP_0          = $(LD_ST40_LINUX_KO) $(LDFLAGS_ST40_LINUX_KO) $1 $2
else
BUILD_APP_0          = $(LD_ST40_LINUX_KO) $(LDFLAGS_ST40_LINUX_KO) $(1:.ko=.o) $2; \
                       $(MODPOST_ST40_LINUX_KO) $(MODPOSTFLAGS_ST40_LINUX_KO) $(1:.ko=.o); \
		       $(CC_ST40_LINUX_KO) -c $(CFLAGS_ST40_LINUX_KO) $(1:.ko=.mod.c) -o $(1:.ko=.mod.lo); \
		       $(LIB_ST40_LINUX_KO) $(LIBFLAGS_ST40_LINUX_KO) $1 $(1:.ko=.o) $(1:.ko=.mod.lo)
endif
LIBRARY_0            = $(OBJDIR_ST40_LINUX_KO)/lib$1.lo
BUILD_LIB_0          = $(LIB_ST40_LINUX_KO) $(LIBFLAGS_ST40_LINUX_KO) $1 $2
RTLD_0               = $(call RTLD_ST40_LINUX_KO,$1,$2,$3)
endif # linux_ko
ifeq ($(OS_0),os21)
ENABLE_ST40_OS21     = 1
BOARD_ST40_OS21      = $(BOARD)
OBJS_0               = $(foreach f,$(1:.c=.$(OBJ_ST40_OS21)),$(OBJDIR_ST40_OS21)/$f)
CPP_0                = $(CPP_ST40_OS21) $(CPPFLAGS_ST40_OS21) $2 > $1
APPLICATION_0        = $(OBJDIR_ST40_OS21)/$1.out
BUILD_APP_0          = $(LD_ST40_OS21) -o $1 $2 $(LDFLAGS_ST40_OS21)
LIBRARY_0            = $(OBJDIR_ST40_OS21)/lib$1.a
BUILD_LIB_0          = $(LIB_ST40_OS21) $(LIBFLAGS_ST40_OS21) $1 $2
RTLD_0               = $(call RTLD_ST40_OS21,$1,$2,$3)
endif # os21
ifeq ($(OS_0),wince)
ENABLE_ST40_WINCE    = 1
export ENABLE_ST40_WINCE
BOARD_ST40_WINCE     = $(BOARD)
OBJS_0               = $(foreach f,$(1:.c=.$(OBJ_ST40_WINCE)),$(OBJDIR_ST40_WINCE)/$f)
CPP_0                = $(CPP_ST40_WINCE) $(CPPFLAGS_ST40_WINCE) $2 > $1
APPLICATION_0        = $(OBJDIR_ST40_WINCE)/$1.exe
BUILD_APP_0          = $(LD_ST40_WINCE) -out:$1 $2 $(LDFLAGS_ST40_WINCE)
LIBRARY_0            = $(OBJDIR_ST40_WINCE)/$1.lib
BUILD_LIB_0          = $(LIB_ST40_WINCE) $(LIBFLAGS_ST40_WINCE) -out:$1 $2
RTLD_0               = $(call RTLD_ST40_WINCE,$1,$2,$3)
endif # wince
endif # st40
ifeq ($(CORE_0),st231)
ifeq ($(OS_0),os21)
ENABLE_ST231_OS21    = 1
BOARD_ST231_OS21     = $(BOARD)
OBJS_0               = $(foreach f,$(1:.c=.$(OBJ_ST231_OS21)),$(OBJDIR_ST231_OS21)/$f)
CPP_0                = $(CPP_ST231_OS21) $(CPPFLAGS_ST231_OS21) $2 > $1
APPLICATION_0        = $(OBJDIR_ST231_OS21)/$1.out
BUILD_APP_0          = $(LD_ST231_OS21) -o $1 $2 $(LDFLAGS_ST231_OS21)
LIBRARY_0            = $(OBJDIR_ST231_OS21)/lib$1.a
BUILD_LIB_0          = $(LIB_ST231_OS21) $(LIBFLAGS_ST231_OS21) $1 $2
RTLD_0               = $(call RTLD_ST231_OS21,$1,$2,$3)
endif # os21
endif # st231

#
# manage build macros etc. for the second CPU
#

ifeq ($(CORE_1),st40)
ifeq ($(OS_1),linux)
ENABLE_ST40_LINUX    = 1
BOARD_ST40_LINUX     = $(BOARD)
OBJS_1               = $(foreach f,$(1:.c=.$(OBJ_ST40_LINUX)),$(OBJDIR_ST40_LINUX)/$f)
CPP_1                = $(CPP_ST40_LINUX) $(CPPFLAGS_ST40_LINUX) $2 > $1
APPLICATION_1        = $(OBJDIR_ST40_LINUX)/$1.out
BUILD_APP_1          = $(LD_ST40_LINUX) -o $1 $2 $(LDFLAGS_ST40_LINUX)
LIBRARY_1            = $(OBJDIR_ST40_LINUX)/lib$1.so
BUILD_LIB_1          = $(LIB_ST40_LINUX) $(LIBFLAGS_ST40_LINUX) $1 $2
RTLD_1               = $(call RTLD_ST40_LINUX,$1,$2,$3)
endif # linux
ifeq ($(OS_1),linux_ko)
ENABLE_ST40_LINUX_KO = 1
BOARD_ST40_LINUX_KO  = $(BOARD)
OBJS_1               = $(foreach f,$(1:.c=.$(OBJ_ST40_LINUX_KO)),$(OBJDIR_ST40_LINUX_KO)/$f)
CPP_1                = $(CPP_ST40_LINUX_KO) $(CPPFLAGS_ST40_LINUX_KO) $2 > $1
APPLICATION_1        = $(OBJDIR_ST40_LINUX_KO)/$1.ko
# we haven't included st40_linux_ko.mak at this point so we can't do the 'proper' check
ifneq (,$(findstring ST40Linux-1.0,$(KERNELDIR_ST40_LINUX_KO)))
BUILD_APP_1          = $(LD_ST40_LINUX_KO) $(LDFLAGS_ST40_LINUX_KO) $1 $2
else
BUILD_APP_1          = $(LD_ST40_LINUX_KO) $(LDFLAGS_ST40_LINUX_KO) $(1:.ko=.o) $2; \
                       $(MODPOST_ST40_LINUX_KO) $(MODPOSTFLAGS_ST40_LINUX_KO) $(1:.ko=.o); \
		       $(CC_ST40_LINUX_KO) -c $(CFLAGS_ST40_LINUX_KO) $(1:.ko=.mod.c) -o $(1:.ko=.mod.lo); \
		       $(LIB_ST40_LINUX_KO) $(LIBFLAGS_ST40_LINUX_KO) $1 $(1:.ko=.o) $(1:.ko=.mod.lo)
endif
LIBRARY_1            = $(OBJDIR_ST40_LINUX_KO)/lib$1.lo
BUILD_LIB_1          = $(LIB_ST40_LINUX_KO) $(LIBFLAGS_ST40_LINUX_KO) $1 $2
RTLD_1               = $(call RTLD_ST40_LINUX_KO,$1,$2,$3)
endif # linux_ko
ifeq ($(OS_1),os21)
ENABLE_ST40_OS21     = 1
BOARD_ST40_OS21      = $(BOARD)
OBJS_1               = $(foreach f,$(1:.c=.$(OBJ_ST40_OS21)),$(OBJDIR_ST40_OS21)/$f)
CPP_1                = $(CPP_ST40_OS21) $(CPPFLAGS_ST40_OS21) $2 > $1
APPLICATION_1        = $(OBJDIR_ST40_OS21)/$1.out
BUILD_APP_1          = $(LD_ST40_OS21) -o $1 $2 $(LDFLAGS_ST40_OS21)
LIBRARY_1            = $(OBJDIR_ST40_OS21)/lib$1.a
BUILD_LIB_1          = $(LIB_ST40_OS21) $(LIBFLAGS_ST40_OS21) $1 $2
RTLD_1               = $(call RTLD_ST40_OS21,$1,$2,$3)
endif # os21
endif # st40
ifeq ($(CORE_1),st231)
ifeq ($(OS_1),os21)
ENABLE_ST231_OS21    = 1
BOARD_ST231_OS21     = $(BOARD)
OBJS_1               = $(foreach f,$(1:.c=_core1.$(OBJ_ST231_OS21)),$(OBJDIR_ST231_OS21)/$f)
CPP_1                = $(CPP_ST231_OS21) $(CPPFLAGS_ST231_OS21) $2 > $1
APPLICATION_1        = $(OBJDIR_ST231_OS21)/$1_core1.out
BUILD_APP_1          = $(LD_ST231_OS21) -o $1 $2 $(LDFLAGS_ST231_OS21)
LIBRARY_1            = $(OBJDIR_ST231_OS21)/lib$1_core1.a
BUILD_LIB_1          = $(LIB_ST231_OS21) $(LIBFLAGS_ST231_OS21) $1 $2
RTLD_1               = $(call RTLD_ST231_OS21,$1,$2,$3)
endif # os21
endif # st231

ifndef OBJS_0
$(error Invalid CPU/OS combination $(CORE_0)/$(OS_0))
endif

ifndef OBJS_1
$(error Invalid CPU/OS combination $(CORE_1)/$(OS_1))
endif

endif # CORE_0
endif # PLATFORM

include $(RPC_ROOT)/src/mkfiles/build.mak
