#
# makefile
#
# Copyright (C) STMicroelectronics Limited 2000, 01. All rights reserved.
#
# Makefile for the mailbox API shell 
#

RPC_ROOT = ../../..
include $(RPC_ROOT)/src/mkfiles/host.mak

#
# configurable macros
#

LIBRARY       = embxshell

COMMON_INCLUDES=../include

SRCS          = embx_buffer.c \
		embx_connect.c \
		embx_debug.c \
		embx_deinit.c \
		embx_handle.c \
		embx_init.c \
		embx_object.c \
		embx_open.c \
		embx_port.c \
		embx_tune.c \
		embx_cache.c \
		embx_sendrec.c \
		embx_support.c \
		embx_transport.c

SRCS_OS21     = embx_os21.c
SRCS_LINUX    = embx_linux.c
SRCS_LINUX_KO = embx_module.c
SRCS_WIN      = embx_win.c
SRCS_SOLARIS  = embx_solaris.c

HDRS	      = $(COMMON_HDRS) $(LOCAL_HDRS)

LOCAL_HDRS    = embxshell.h \
		debug_ctrl.h

COMMON_HDRS   = $(COMMON_INCLUDES)/embx_types.h \
		$(COMMON_INCLUDES)/embx_typesP.h \
		$(COMMON_INCLUDES)/embx.h \
		$(COMMON_INCLUDES)/embxP.h \
		$(COMMON_INCLUDES)/embx_debug.h \
		$(COMMON_INCLUDES)/embx_osheaders.h \
		$(COMMON_INCLUDES)/embx_osinterface.h

LOCAL_CFLAGS  = -I$(COMMON_INCLUDES)

#
# derived macros
#

OBJS_IA32_LINUX    = $(foreach f, $(patsubst %.c,%.$(OBJ_IA32_LINUX),$(SRCS) $(SRCS_LINUX)), \
				$(OBJDIR_IA32_LINUX)/$f)
OBJS_IA32_WINNT    = $(foreach f, $(patsubst %.c,%.$(OBJ_IA32_WINNT),$(SRCS) $(SRCS_WIN)), \
				$(OBJDIR_IA32_WINNT)/$f)
OBJS_SPARC_SOLARIS = $(foreach f, $(patsubst %.c,%.$(OBJ_SPARC_SOLARIS),$(SRCS) $(SRCS_SOLARIS)), \
				$(OBJDIR_SPARC_SOLARIS)/$f)
OBJS_ST40_LINUX    = $(foreach f, $(patsubst %.c,%.$(OBJ_ST40_LINUX),$(SRCS) $(SRCS_LINUX)), \
				$(OBJDIR_ST40_LINUX)/$f)
OBJS_ST40_LINUX_KO = $(foreach f, $(patsubst %.c,%.$(OBJ_ST40_LINUX_KO),$(SRCS) $(SRCS_LINUX) $(SRCS_LINUX_KO)), \
				$(OBJDIR_ST40_LINUX_KO)/$f)
OBJS_ST40_OS21     = $(foreach f, $(patsubst %.c,%.$(OBJ_ST40_OS21),$(SRCS) $(SRCS_OS21)), \
				$(OBJDIR_ST40_OS21)/$f)
OBJS_ST40_WINCE    = $(foreach f, $(patsubst %.c,%.$(OBJ_ST40_WINCE),$(SRCS) $(SRCS_WIN)), \
				$(OBJDIR_ST40_WINCE)/$f)
OBJS_ST231_OS21    = $(foreach f, $(patsubst %.c,%.$(OBJ_ST231_OS21),$(SRCS) $(SRCS_OS21)), \
				$(OBJDIR_ST231_OS21)/$f)

$(OBJS_IA32_LINUX) $(OBJS_IA32_WINNT) \
$(OBJS_ST40_LINUX) $(OBJS_ST40_LINUX_KO) $(OBJS_ST40_OS21) $(OBJS_ST40_WINCE) \
$(OBJS_ST231_OS21) $(OBJS_SPARC_SOLARIS) : $(HDRS)

#
# explicit rules
#

include $(RPC_ROOT)/src/mkfiles/build.mak

all: $(MAKE_DIRS) $(MAKE_TARGETS)

clean:
	-$(RM) $(call DOSCMD,$(CLEAN_DIRS))

#
# manage distributions
#
DIST_SRC = $(SRCS) $(SRCS_LINUX) $(SRCS_LINUX_KO) $(SRCS_WIN) $(SRCS_OS21) $(SRCS_SOLARIS) $(LOCAL_HDRS) makefile

include $(RPC_ROOT)/src/mkfiles/install.mak

