#
# target.mak
#
# Copyright (C) STMicroelectronics Limited 2002. All rights reserved.
#
# Test harness include file.
#

HARNESS_INCLUDES = $(RPC_TEST)/harness

RPC_ROOT    = $(RPC_TEST)/../../../..
include $(RPC_ROOT)/src/mkfiles/host.mak
include $(RPC_ROOT)/src/mkfiles/platform.mak

LOCAL_CFLAGS += -I$(HARNESS_INCLUDES) -I$(RPC_ROOT)/src/embx/include $(CONF_CFLAGS)

ifdef ENABLE_EMBXSHMC
EMBXSHM_LIB=embxshmc
LOCAL_CFLAGS += -DENABLE_EMBXSHMC
else
EMBXSHM_LIB=embxshm
endif

ifdef PLATFORM
ifeq ($(OS_0),linux)
DISABLE_TEST=1
endif
ifeq ($(OS_0),linux_ko)
DISABLE_TEST=1
endif
endif
