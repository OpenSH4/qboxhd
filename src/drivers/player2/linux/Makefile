#KDIR=/opt/STM/STLinux-2.3/devkit/build/build-mb442/kernel
#KDIR_SRC_PATH=/opt/STM/STLinux-2.3/devkit/sources/kernel/havana-linux-sh4
#ARCH = sh
#CROSS_COMPILE = sh4-linux-
#PWD = $(shell pwd)

DRIVER_TOPDIR?=..
TREE_ROOT?=..
#PLAYER2_PATH=/home/utente/space/qboxhd_drivers/player2/trunk
#STMFB_PATH=/opt/STM/STLinux-2.3/devkit/build/build-mb442/stmfb
#INCLUDE_PATH=/home/utente/space/qboxhd_drivers/include/trunk
#AVS_PATH=/home/utente/space/qboxhd_drivers/avs/trunk
#PTI_PATH=/home/utente/space/qboxhd_drivers/pti/trunk/
#FE_PATH=/home/utente/space/qboxhd_drivers/frontends/trunk
#DELAYER_PATH=/home/utente/space/qboxhd_drivers/fpga/delayer
CFLAGS+=-I$(CONFIG_KERNEL_BUILD)/include2 \
	-I$(KDIR)/include \
	-I$(KDIR)/include/linux \
	-I$(KDIR_SRC_PATH)/drivers/media/dvb/dvb-core 				\
	-I$(KDIR_SRC_PATH)/include -D__KERNEL__ \
        -I$(DRIVER_TOPDIR)/include/

CFLAGS+=-I$(TREE_ROOT)/include/player2			   					\
	-I$(PLAYER2_PATH)/linux/components/inline         				\
	-I$(PLAYER2_PATH)/components/include              				\
	-I$(PLAYER2_PATH)/components/infrastructure       				\
	-I$(PLAYER2_PATH)/linux/include									\
	-I$(PLAYER2_PATH)/linux/drivers/media/dvb/stm/allocator 		\
	-I$(PLAYER2_PATH)/linux/drivers/media/dvb/stm/h264_preprocessor \
	-I$(PLAYER2_PATH)/linux/drivers/media/dvb/stm/dvb 				\
	-I$(PLAYER2_PATH)/linux/drivers/media/sysfs/stm 				\
	-I$(PLAYER2_PATH)/linux/drivers/stm/st_relay 					\
	-I$(PLAYER2_PATH)/linux/drivers/stm/mmelog 						\
	-I$(PLAYER2_PATH)/linux/drivers/sound/ksound 					\
	-I$(PLAYER2_PATH)/linux/drivers/sound/pseudocard

CFLAGS+=-I$(PLAYER2_PATH)/player/class_definitions	\
	-I$(PLAYER2_PATH)/player/buffer                   \
	-I$(PLAYER2_PATH)/player/demultiplexor            \
	-I$(PLAYER2_PATH)/player/collator                 \
	-I$(PLAYER2_PATH)/player/frame_parser             \
	-I$(PLAYER2_PATH)/player/codec                    \
	-I$(PLAYER2_PATH)/player/standards                \
	-I$(PLAYER2_PATH)/player/manifestor               \
	-I$(PLAYER2_PATH)/player/player/                  \
	-I$(PLAYER2_PATH)/player/output_coordinator       \
	-I$(PLAYER2_PATH)/player/output_timer

CFLAGS+=-I$(STMFB_PATH)
CFLAGS+=-I$(STMFB_PATH)/include
CFLAGS+=-I$(STMFB_PATH)/Linux/include
CFLAGS+=-I$(STMFB_PATH)/Linux/media/video

CFLAGS+=-I$(INCLUDE_PATH)
CFLAGS+=-I$(INCLUDE_PATH)/player2
CFLAGS+=-I$(INCLUDE_PATH)/multicom
CFLAGS+=-I$(INCLUDE_PATH)/player2/transformers
CFLAGS+=-I$(PTI_PATH)
CFLAGS+=-I$(DELAYER_PATH)
CFLAGS+=-I$(AVS_PATH)
CFLAGS+=-I$(DELAYER_PATH)
CFLAGS+=-I$(FE_PATH)/cx24116
CFLAGS+=-I$(FE_PATH)/avl2108
CFLAGS+=-I$(FE_PATH)/zl10353
CFLAGS+=-I$(FE_PATH)/tda10048
CFLAGS+=-I$(FE_PATH)/tda18218
CFLAGS+=-I$(FE_PATH)/tda10023

# promote CONFIG_RELAY to a CFLAGS define so that it can be observed by C++
# code as well and appropriate inlining performed at the earliest possible
# point.
ifdef CONFIG_RELAY
CFLAGS+=-DCONFIG_RELAY=1
endif

# CONFIG_DUAL_DISPLAY is a temporary hack to convert the existing
# picture-in-picture and speaker-in-speaker support into two
# independant displays.
#CFLAGS+=-DCONFIG_DUAL_DISPLAY

CFLAGS+=-I$(KDIR)/include/linux

# promote CONFIG_MONITOR to a CFLAGS define so that it can be observed by C++
# code as well and appropriate inlining performed at the earliest possible
# point.
#ifdef CONFIG_MONITOR
CFLAGS+=-DCONFIG_MONITOR=1
#endif

# Finally pick up the original system headers, these must be near the '
# back' of the path to ensure the kernel headers are taken in 
# preference to any system supplied ones. Items such as the MME header 
# files (XXXX_DecoderTypes.h) form part of the system headers.
CFLAGS += -I$(DRIVER_TOPDIR)/include/player2

CFLAGS += -D__SH4__       

CFLAGS += -DREPORT -D__TDT__
#CFLAGS += -D__TDT__

obj-y += drivers/media/dvb/stm/mpeg2_hard_host_transformer/
obj-y += drivers/media/dvb/stm/dvb/
obj-y += drivers/media/dvb/stm/backend/
obj-y += drivers/media/dvb/stm/allocator/
obj-y += drivers/media/dvb/stm/h264_preprocessor/
obj-y += drivers/media/video/stm/
obj-y += drivers/media/sysfs/stm/
obj-y += drivers/sound/ksound/
obj-y += drivers/sound/kreplay/
obj-y += drivers/sound/pseudocard/
obj-y += drivers/sound/silencegen/
obj-y += drivers/stm/

obj-y += dummy.o

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c

