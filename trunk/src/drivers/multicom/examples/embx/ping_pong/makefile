#
# makefile
#
# Copyright (C) STMicroelectronics Limited 2005. All rights reserved.
#
# Makefile for mailbox 'ping pong' test
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

APP_NAME   = ping_pong

APP_MASTER = $(APP_NAME)
APP_SLAVE  = $(APP_NAME)_audio

SRCS_MASTER = $(APP_NAME)_master.c
SRCS_SLAVE  = $(APP_NAME)_slave.c

include ../../harness/harness.mak
