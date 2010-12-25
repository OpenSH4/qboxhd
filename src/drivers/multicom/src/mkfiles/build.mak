#
# mkfiles/build.mak
#
# Copyright (C) STMicroelectronics Limited 2005. All rights reserved.
#
# Meta-include file to pull in all the build rules
#

include $(RPC_ROOT)/src/mkfiles/ia32_linux.mak
include $(RPC_ROOT)/src/mkfiles/ia32_winnt.mak
include $(RPC_ROOT)/src/mkfiles/st40_linux.mak
include $(RPC_ROOT)/src/mkfiles/st40_linux_ko.mak
include $(RPC_ROOT)/src/mkfiles/st40_os21.mak
include $(RPC_ROOT)/src/mkfiles/st40_wince.mak
include $(RPC_ROOT)/src/mkfiles/st231_os21.mak
include $(RPC_ROOT)/src/mkfiles/sparc_solaris.mak
