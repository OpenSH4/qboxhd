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
CC_ST40 = $(error CC_ST20 is obsolete)
CC_ST220 = $(error CC_ST20 is obsolete)

ifndef PLATFORM
ifndef DISABLE_PLATFORM_WARNINGS
$(warning PLATFORM is not set, skipping RPC test)
endif
DISABLE_TEST=1
else

ifdef ENABLE_RPC
ENABLE_RPCCC = $(ENABLE_RPC)
#ENABLE_RPC_SERVER = $(ENABLE_RPC)
endif

ifdef NO_ST40_LINUX
NO_ST40_LINUX_MASTER=1
NO_ST40_LINUX_SLAVE=1
endif
ifdef NO_ST40_LINUX_MASTER
ifeq ($(OS_0),linux)
DISABLE_TEST=1
endif
ifeq ($(OS_0),linux_ko)
DISABLE_TEST=1
endif
endif
ifdef NO_ST40_LINUX_SLAVE
ifeq ($(OS_1),linux)
DISABLE_TEST=1
endif
ifeq ($(OS_1),linux_ko)
DISABLE_TEST=1
endif
endif

include $(RPC_ROOT)/src/mkfiles/platform.mak

LOCAL_CFLAGS += -I$(HARNESS_INCLUDES) -I$(RPC_ROOT)/src/embx/include $(CONF_CFLAGS)

ifdef ENABLE_EMBXSHMC
EMBXSHM_LIB=embxshmc
EMBXSHM_PTRWARP_ST40=0x80000000
LOCAL_CFLAGS += -DENABLE_EMBXSHMC
else
EMBXSHM_LIB=embxshm
EMBXSHM_PTRWARP_ST40=0x60000000
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

ifdef ENABLE_ST40_LINUX_RAMDISK

ifdef STLINUXROOT
# Need to substitute absolute paths in the RAM filesystem list file
GENERATED = $(RPC_TEST)/harness/ramfs_generated.lst
RAMFS_LIST_ST40_LINUX = $(GENERATED)

$(GENERATED): $(RPC_TEST)/harness/ramfs20.lst
	sed s:/opt/STM/STLinux-2.0:$(STLINUXROOT): $< > $@
else
ifeq ($(DIST_ST40_LINUX),$(DIST_ST40_LINUX_1_0))
RAMFS_LIST_ST40_LINUX = $(RPC_TEST)/harness/ramfs.lst
else
ifeq ($(DIST_ST40_LINUX),$(DIST_ST40_LINUX_2_0))
RAMFS_LIST_ST40_LINUX = $(RPC_TEST)/harness/ramfs20.lst
else
ifeq ($(DIST_ST40_LINUX),$(DIST_ST40_LINUX_2_2))
RAMFS_LIST_ST40_LINUX = $(RPC_TEST)/harness/ramfs22.lst
else
RAMFS_LIST_ST40_LINUX = $(RPC_TEST)/harness/ramfs23.lst
endif
endif
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
		-m $(MODULES_ROOT)/lib/linux/st40/rpc_userver.ko \
                $(LOCAL_ST40_MODULES)\
		$(TESTS_ST40_LINUX) \
		$@

ifeq ($(OS_0),linux)
TESTS_ST40_LINUX += -p $(call APPLICATION_0,$(TEST))
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


ifeq ($(PLATFORM),mediaref)
RTLD_KERNEL_ST40_LINUX = $(DIST_ST40_LINUX)/devkit/sh4/target/boot/vmlinux-st-mediaref
RTLD_RAMDISKADDR_ST40_LINUX = 0x88800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttySC0,115200 mem=96m bigphysarea=1024 root=/dev/ram0 ramdisk_size=8192 ip=192.168.1.130::::::off
EMBXMAILBOX_MODARGS=,mailbox0=0x0bb150000:112:st40:activate
EMBXSHM_MODARGS=,mailbox0=shm:0:3:0xc0000000:0:64:64:0:512,empi0=0xbb130000:0xbb150000:0xbb190000:0x80000:0x40000
endif # PLATFORM is mediaref

ifeq ($(PLATFORM),mb376)
RTLD_KERNEL_ST40_LINUX = $(DIST_ST40_LINUX)/devkit/sh4/target/boot/vmlinux-st-sti5528eval
RTLD_RAMDISKADDR_ST40_LINUX = 0x84800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttySC0,115200 mem=64m bigphysarea=1024 root=/dev/ram0 ramdisk_size=8192 ide0=noprobe ide1=noprobe hda=none hdb=none hdc=none hdd=none
EMBXMAILBOX_MODARGS = ,mailbox0=0xb9160000:140:set2:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024
endif # PLATFORM is mb376

ifeq ($(PLATFORM),mb376_reversed)
RTLD_KERNEL_ST40_LINUX = $(DIST_ST40_LINUX)/devkit/sh4/target/boot/vmlinux-st-sti5528eval
RTLD_RAMDISKADDR_ST40_LINUX = 0x84800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttySC0,115200 mem=64m bigphysarea=1024 root=/dev/ram0 ramdisk_size=8192 ide0=noprobe ide1=noprobe hda=none hdb=none hdc=none hdd=none
EMBXMAILBOX_MODARGS = ,mailbox0=0xb9160000:140:set2:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024
endif # PLATFORM is mb376

ifeq ($(PLATFORM),mb379)
RTLD_KERNEL_ST40_LINUX = $(DIST_ST40_LINUX)/devkit/sh4/target/boot/vmlinux-st-stm8000demo
RTLD_RAMDISKADDR_ST40_LINUX = 0x88800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS1,115200 mem=48m bigphysarea=1024 root=/dev/ram0 ramdisk_size=8192 ip=192.168.1.130::::::off
EMBXMAILBOX_MODARGS = ,"mailbox0=0xb0200000:208:set1:locks mailbox1=0xb0210000:0:locks"
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024
endif # PLATFORM is mb379

ifeq ($(PLATFORM),mb392)
EMBXSHM_WARPRANGE_MODARGS= :0xa0000000:0x20000000
RTLD_KERNEL_ST40_LINUX = $(DIST_ST40_LINUX)/devkit/sh4/target/boot/vmlinux-st-st220eval
#RTLD_KERNEL_ST40_LINUX = /home/butch/users/thompsond/workspace/linux-st40-2.6/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x88800000
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS1,115200 mem=48m bigphysarea=1024 root=/dev/ram0 ramdisk_size=8192 ip=164.129.14.180::::::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xb0200000:208:set1:locks,mailbox1=0xb0210000:0:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024$(EMBXSHM_WARPRANGE_MODARGS)
endif # PLATFORM is mb392

ifeq ($(PLATFORM),mb411)
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x84800000
# oot=/dev/nfs nfsroot=164.129.15.35:/opt/STM/STLinux-2.0/devkit/sh4/target nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS0,115200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=8192 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xb9211000:136:set2:locks,mailbox1=0xb9212000:0:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024
endif # PLATFORM is mb411

ifeq ($(PLATFORM),stb7100ref)
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
RTLD_RAMDISKADDR_ST40_LINUX = 0x84800000
# oot=/dev/nfs nfsroot=164.129.15.35:/opt/STM/STLinux-2.0/devkit/sh4/target nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
RTLD_COMMANDLINE_ST40_LINUX = console=ttyAS0,19200 mem=48m bigphysarea=1024 \
                              root=/dev/ram0 ramdisk_size=8192 \
			      nwhwconf=device:eth0,hwaddr:00:80:E1:12:02:1C \
			      ip=164.129.14.180:164.129.15.35:164.129.8.253:255.255.248.0:scrutock::off
EMBXMAILBOX_MODARGS = ,mailbox0=0xb9211000:136:set2:locks,mailbox1=0xb9212000:0:locks
EMBXSHM_MODARGS = ,mailbox0=shm:0:3:$(EMBXSHM_PTRWARP_ST40):0:64:64:0:1024
endif # PLATFORM is mb411

# override RTLD_KERNEL_ST40_LINUX if vmlinux exists in the KERNELDIR_ST40_LINUX_KO directory
ifneq (,$(wildcard $(KERNELDIR_ST40_LINUX_KO)/vmlinux))
RTLD_KERNEL_ST40_LINUX = $(KERNELDIR_ST40_LINUX_KO)/vmlinux
endif

else # ENABLE_ST40_LINUX_RAMDISK

st40_linux_ramdisk :

endif # ENABLE_ST40_LINUX_RAMDISK

endif # TARGET

# the clean rule *must* be in target.mak since target.mak is included by *all* RPC tests
# while harness.mak is only included by the C -> runtime loader based tests.
clean :
	-$(RM) $(call DOSCMD,$(CLEAN_DIRS) $(LITTER) *.passed *.stubs.cpped *.stubs.c)

