#
# makefile
#
# Copyright (C) STMicroelectronics Limited 2003. All rights reserved.
#
# Makefile for EMBX three cpu example
#

ifdef OS_0
ifneq ($(OS_0),os21)
$(error This example can only be run with OS21 master and slave configurations)
endif
endif

ifdef OS_1
ifneq ($(OS_1),os21)
$(error This example can only be run with OS21 master and slave configurations)
endif
endif

APP_NAME   = three_cpu_hello

APP_MASTER = $(APP_NAME)
APP_SLAVE  = $(APP_NAME)_audio
APP_VIDEO   = $(APP_NAME)_video

SRCS_MASTER = $(APP_NAME)_master.c
SRCS_SLAVE  = $(APP_NAME)_slave.c

include ../../harness/harness.mak

# handle CPU 2 as a variant of CPU 1

example : app_video

app_video : BOARD=$(subst audio,video,$(BOARD_1))
app_video : $(call APPLICATION_1,$(APP_VIDEO))

$(call APPLICATION_1,$(APP_VIDEO)) : $(call OBJS_1,$(SRCS_SLAVE))
	@$(ECHO) +++ Link $(MACHINE_1) application for CPU 2 [$(call APPLICATION_1,$(APP_VIDEO))] +++
	$(call BUILD_APP_1,$(call APPLICATION_1,$(APP_VIDEO)),$(call OBJS_1,$(SRCS_SLAVE)))
