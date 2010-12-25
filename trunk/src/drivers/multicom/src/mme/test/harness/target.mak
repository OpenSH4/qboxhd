#
# target.mak
#
# Copyright (C) STMicroelectronics Limited 2002. All rights reserved.
#
# Test harness include file.
#

RPC_ROOT    = $(RPC_TEST)/../../..
include $(RPC_ROOT)/src/mkfiles/host.mak

MODULES_ROOT = $(RPC_ROOT)

HARNESS_INCLUDES = $(RPC_TEST)/harness

CC_ST20 = $(error CC_ST20 is obsolete)
CC_ST40 = $(error CC_ST40 is obsolete)
CC_ST220 = $(error CC_ST220 is obsolete)

ifndef PLATFORM
ifndef DISABLE_PLATFORM_WARNINGS
$(warning PLATFORM is not set, skipping MME test)
endif
DISABLE_TEST=1
else

#
# Modifiers that take place *before* we include platform
#

ifdef ENABLE_RPC
ENABLE_RPCCC = $(ENABLE_RPC)
#ENABLE_RPC_SERVER = $(ENABLE_RPC)
endif

ifdef NO_ST40_LINUX_MASTER
ifeq ($(OS_0),linux)
DISABLE_TEST=1
endif
ifeq ($(OS_0),linux_ko)
DISABLE_TEST=1
endif
endif

# Allow either KERNELDIR_ST40_LINUX_KO or O to be used
# to specify where to find the Linux kernel image
KERNELDIR_ST40_LINUX_KO ?= $(O)

#
# Customize the build for the appropriate platform
#

include $(RPC_ROOT)/src/mkfiles/platform.mak

#
# Modifiers that take place *after* we include platform
#

ifdef ENABLE_EMBXSHMC
EMBXSHM_LIB=embxshmc
EMBXSHM_PTRWARP_ST40=0x80000000
LOCAL_CFLAGS += -DENABLE_EMBXSHMC
else
EMBXSHM_LIB=embxshm
EMBXSHM_PTRWARP_ST40=0x60000000
endif

ifdef DISABLE_SLAVE
APPLICATION_1 =
RTLD_1 = sleep 1
endif

# these test use a mixture of the RPC 1.x os_abstractions.h (which now
# live in the examples directory) and the EMBX 2.0 embx_osinterface.h.
# use of the later ought to be eliminated.
LOCAL_CFLAGS += -I$(HARNESS_INCLUDES) \
	        -I$(RPC_ROOT)/src/embx/include \
		-I$(RPC_ROOT)/examples/harness \
		$(CONF_CFLAGS)

ifeq ($(DIST_ST40_LINUX_2_0),$(DIST_ST40_LINUX))
LOCAL_CFLAGS_ST40_LINUX_KO += \
	-DKBUILD_MODNAME=$(LIBRARY) \
	-DKBUILD_BASENAME=$(subst -,_,$(*F))
else
# STLinux 2.2/2.3
LOCAL_CFLAGS_ST40_LINUX_KO += \
	-D"KBUILD_STR(s)=\#s" \
	-D"KBUILD_MODNAME=KBUILD_STR(test)" \
	-D"KBUILD_BASENAME=KBUILD_STR($(*F))" \
	$(CONF_CFLAGS)
endif

MASTER = $(call TOUPPER,$(CORE_0)_$(OS_0))
SLAVE  = $(call TOUPPER,$(CORE_1)_$(OS_1))

TEST_PASSED  = $(PLATFORM)_$(CORE_0)_$(OS_0)_$(TEST).passed
MASTER_STUBS = $(PLATFORM)_$(CORE_0)_$(OS_0)_master.stubs
SLAVE_STUBS  = $(PLATFORM)_$(CORE_1)_$(OS_1)_slave.stubs

ifdef ENABLE_ST40_LINUX
ENABLE_ST40_LINUX_RAMDISK=1
RAMDISK_ST40_LINUX=$(OBJDIR_ST40_LINUX)/$(TEST).img
endif
ifdef ENABLE_ST40_LINUX_KO
ENABLE_ST40_LINUX_RAMDISK=1
RAMDISK_ST40_LINUX=$(OBJDIR_ST40_LINUX)/$(TEST)_ko.img
endif

endif # TARGET


ifdef ENABLE_ST40_LINUX_RAMDISK

ifdef STLINUXROOT
# Need to substitute absolute paths in the RAM filesystem list file
GENERATED = $(RPC_TEST)/harness/ramfs_generated.lst
RAMFS_LIST_ST40_LINUX = $(GENERATED)

$(RAMFS_LIST_ST40_LINUX): $(RPC_TEST)/harness/ramfs22.lst
	sed s:/opt/STM/STLinux-2.0:$(STLINUXROOT): $< > $@
else
ifeq ($(DIST_ST40_LINUX),$(DIST_ST40_LINUX_2_2))
RAMFS_LIST_ST40_LINUX = $(RPC_TEST)/harness/ramfs22.lst
else
RAMFS_LIST_ST40_LINUX = $(RPC_TEST)/harness/ramfs23.lst
endif
endif

st40_linux_ramdisk : $(RAMDISK_ST40_LINUX)

$(RAMDISK_ST40_LINUX) : $(SRCS) $(HDRS) makefile $(RAMFS_LIST_ST40_LINUX)
	linuxshmkimg \
		-R $(DIST_ST40_LINUX)/devkit/sh4/target \
		-c $(RAMFS_LIST_ST40_LINUX) \
		-d $(RPC_TEST)/harness/ramfs \
		-m $(MODULES_ROOT)/lib/linux/st40/embxshell.ko \
		-m $(MODULES_ROOT)/lib/linux/st40/embxmailbox.ko$(EMBXMAILBOX_MODARGS) \
		-m $(MODULES_ROOT)/lib/linux/st40/$(EMBXSHM_LIB).ko$(EMBXSHM_MODARGS) \
		-m $(MODULES_ROOT)/lib/linux/st40/mme_host.ko$(MMEHOST_MODARGS) \
                $(LOCAL_ST40_MODULES)\
		$(TESTS_ST40_LINUX) \
		$@

ifdef ENABLE_ST40_LINUX
ifndef DISABLE_SLAVE
MMEHOST_MODARGS = ,transport0=shm
endif
endif

ifeq ($(OS_0),linux)
TESTS_ST40_LINUX += -p $(call APPLICATION_0,$(TEST)) \
                    -f $(RPC_ROOT)/lib/linux/st40/libmme_host.so \
		    -f $(RPC_TEST)/harness/$(OBJDIR_ST40_LINUX)/libharness.so
endif
ifeq ($(OS_0),linux_ko)
TESTS_ST40_LINUX += -m $(call APPLICATION_0,$(TEST)) -p $(RPC_TEST)/harness/rpctest.sh
endif
ifeq ($(OS_1),linux)
TESTS_ST40_LINUX += -p $(call APPLICATION_1,$(TEST))
endif
ifeq ($(OS_1),linux_ko)
TESTS_ST40_LINUX += -m $(call APPLICATION_1,$(TEST)) -p $(RPC_TEST)/harness/rpctest.sh
endif

ifeq ($(PLATFORM),mb411)
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x84800000
# oot=/dev/nfs nfsroot=164.129.15.35:/opt/STM/STLinux-2.0/devkit/sh4/target nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS0,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=16384 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xb9211000:136:set2:locks,mailbox1=0xb9212000:0:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024
endif # PLATFORM is mb411

ifeq ($(PLATFORM),mb442)
EMBXSHM_WARPRANGE_MODARGS= :0xa0000000:0x20000000
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x84800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS0,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=16384 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xb9211000:136:set2:locks,mailbox1=0xb9212000:0:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024$(EMBXSHM_WARPRANGE_MODARGS)
endif # PLATFORM is mb442

ifeq ($(PLATFORM),mb442se)
EMBXSHM_WARPRANGE_MODARGS= :0x40000000:0x20000000
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x80800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS0,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=16384 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0x19211000:136:set2:locks,mailbox1=0x19212000:0:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024$(EMBXSHM_WARPRANGE_MODARGS)
endif # PLATFORM is mb442se

ifeq ($(PLATFORM),mb519)
EMBXSHM_WARPRANGE_MODARGS= :0xa0000000:0x20000000
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x88800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS1,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=16384 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xfd800000:44:set2,mailbox1=0xfd801000:0
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024$(EMBXSHM_WARPRANGE_MODARGS)
endif # PLATFORM is mb519

ifeq ($(PLATFORM),mb519se)
EMBXSHM_WARPRANGE_MODARGS= :0x40000000:0x10000000:0x80000000:0x10000000
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x80800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS1,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=16384 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xfd800000:44:set2:locks,mailbox1=0xfd801000:0:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024$(EMBXSHM_WARPRANGE_MODARGS)
endif # PLATFORM is mb519se

ifeq ($(PLATFORM),mb618)
EMBXSHM_WARPRANGE_MODARGS= :0xac000000:0x10000000
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x8c800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS1,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=16384 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xfe212000:137:set2
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024$(EMBXSHM_WARPRANGE_MODARGS)
endif # PLATFORM is mb618

ifeq ($(PLATFORM),mb618se)
EMBXSHM_WARPRANGE_MODARGS= :0x40000000:0x10000000
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x80800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS1,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=16384 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xfe212000:137:set2
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024$(EMBXSHM_WARPRANGE_MODARGS)
endif # PLATFORM is mb618se

# override RTLD_KERNEL_ST40_LINUX if vmlinux exists in the KERNELDIR_ST40_LINUX_KO directory
ifneq (,$(wildcard $(KERNELDIR_ST40_LINUX_KO)/vmlinux))
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
endif

else # ENABLE_ST40_LINUX_RAMDISK

st40_linux_ramdisk :

endif # ENABLE_ST40_LINUX_RAMDISK

# the clean rule *must* be in target.mak since target.mak is included by *all* RPC tests
# while harness.mak is only included by the C -> runtime loader based tests.
clean :
	-$(RM) $(call DOSCMD, $(RAMDISK_ST40_LINUX) $(CLEAN_DIRS) $(LITTER) *.passed *.stubs.cpped *.stubs.c)

