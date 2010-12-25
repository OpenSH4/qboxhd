#
# subdir.mak
#
# Copyright (C) STMicroelectronics Limited 2002. All rights reserved.
#
# Test harness include file.
#

RPC_ROOT    = $(RPC_TEST)/../../..

include $(RPC_ROOT)/src/mkfiles/host.mak
include $(RPC_ROOT)/src/mkfiles/subdir.mak

ifndef DISABLE_INSTALL
DIST_SRC    = makefile
else
LOCAL_MAKEFLAGS = DISABLE_INSTALL=1
endif

include $(RPC_ROOT)/src/mkfiles/install.mak
