#
# makefile
#
# Copyright (C) STMicroelectronics Ltd. 2002
#
# Makefile for shared subtree
#

RPC_ROOT = ..

include $(RPC_ROOT)/src/mkfiles/host.mak
include $(RPC_ROOT)/src/mkfiles/platform.mak

SUBDIRS = mkfiles tools embx $(wildcard mme) $(wildcard rpc)

include $(RPC_ROOT)/src/mkfiles/subdir.mak

DIST_SRC       = makefile

include $(RPC_ROOT)/src/mkfiles/install.mak

