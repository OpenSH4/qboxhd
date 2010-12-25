#
# generic_test.mak
#
# Copyright (C) STMicroelectronics Limited 2002. All rights reserved.
#
# Makefile fragment for EMBX Shell framework tests
#

include $(RPC_ROOT)/src/mkfiles/host.mak

TEST_PATH := $(subst $(subst _, ,_),\ ,$(shell $(PWD)))
TEST_PATH := $(subst \,/,$(TEST_PATH))
TESTNAME  := $(notdir $(TEST_PATH))

#
# configurable macros
#
COMMON_INCLUDES = $(RPC_ROOT)/src/embx/include

APPLICATION     = shelltest

TESTDIR         = $(RPC_ROOT)/src/embx/tests/embxshell_tests
vpath %.c $(TESTDIR)

HDRS            = $(COMMON_INCLUDES)/embx.h             \
		  $(COMMON_INCLUDES)/embx_types.h       \
		  $(COMMON_INCLUDES)/embx_osinterface.h \
		  $(COMMON_INCLUDES)/embx_osheaders.h   \
		  $(COMMON_INCLUDES)/embx_debug.h

LOCAL_CFLAGS    = -I$(COMMON_INCLUDES)
LOCAL_LDFLAGS_SPARC_SOLARIS = -lembxloopback

#
# derived macros
#

OBJS_SPARC_SOLARIS = $(OBJDIR_SPARC_SOLARIS)/$(TESTNAME).$(OBJ_SPARC_SOLARIS) \
                     $(OBJDIR_SPARC_SOLARIS)/test_main.$(OBJ_SPARC_SOLARIS)

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

DIST_SRC = $(TESTNAME).c makefile

include $(RPC_ROOT)/src/mkfiles/install.mak
