#
# arch/sh/boot/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1999 Stuart Menefy
#

# Set these to dummy values if not defined to avoid error messages.
CONFIG_MEMORY_START     ?= 0
CONFIG_ZERO_PAGE_OFFSET ?= 0

#
# Assign safe dummy values if these variables are not defined,
# in order to suppress error message.
#
CONFIG_PAGE_OFFSET	?= 0x80000000
CONFIG_MEMORY_START	?= 0x0c000000
CONFIG_BOOT_LINK_OFFSET	?= 0x00800000
CONFIG_ZERO_PAGE_OFFSET	?= 0x00001000

MKIMAGE := $(srctree)/scripts/mkuboot.sh

export CONFIG_PAGE_OFFSET CONFIG_MEMORY_START CONFIG_BOOT_LINK_OFFSET \
       CONFIG_ZERO_PAGE_OFFSET

targets := zImage vmlinux.srec uImage uImage.srec
subdir- := compressed

$(obj)/zImage: $(obj)/compressed/vmlinux FORCE
	$(call if_changed,objcopy)
	@echo '  Kernel: $@ is ready'

$(obj)/compressed/vmlinux: FORCE
	$(Q)$(MAKE) $(build)=$(obj)/compressed $@

ifeq ($(CONFIG_32BIT),y)
KERNEL_LOAD	:= $(shell /bin/bash -c 'printf "0x%8x" \
		     $$[$(CONFIG_PAGE_OFFSET)  + \
			$(CONFIG_BOOT_LINK_OFFSET) + \
			$(CONFIG_ZERO_PAGE_OFFSET)]')
else
KERNEL_LOAD	:= $(shell /bin/bash -c 'printf "0x%8x" \
		     $$[$(CONFIG_PAGE_OFFSET)  + \
			$(CONFIG_MEMORY_START) + \
			$(CONFIG_ZERO_PAGE_OFFSET)]')
endif

KERNEL_ENTRY	:= $(shell /bin/bash -c 'printf "0x%8x" \
		     $$[$(KERNEL_LOAD)+0x1000]')

quiet_cmd_uimage = UIMAGE  $@
quiet_cmd_uimage_sign = UVSIMAGE  $@
cmd_uimage_sign = $(CONFIG_SHELL) $(MKIMAGE) -cbin mkimage_sign -A sh -O linux \
		   -T kernel -C gzip -a $(KERNEL_LOAD) -e $(KERNEL_ENTRY) \
		   -n 'Linux-$(KERNELRELEASE)' -b $(CONFIG_SH_BOARD_NAME) \
		   -d $< $@

ifdef CONFIG_SH_BOARD_NAME
ifneq ($(CONFIG_SH_BOARD_NAME),"")
targets += uvsImage
cmd_uimage = $(CONFIG_SHELL) $(MKIMAGE) -cbin mkimage_bname -A sh -O linux -T kernel \
		   -C gzip -a $(KERNEL_LOAD) -e $(KERNEL_ENTRY) \
		   -n 'Linux-$(KERNELRELEASE)' -b $(CONFIG_SH_BOARD_NAME) -d $< $@
else
cmd_uimage = $(CONFIG_SHELL) $(MKIMAGE) -A sh -O linux -T kernel \
		   -C gzip -a $(KERNEL_LOAD) -e $(KERNEL_ENTRY) \
		   -n 'Linux-$(KERNELRELEASE)' -d $< $@
endif
else
cmd_uimage = $(CONFIG_SHELL) $(MKIMAGE) -A sh -O linux -T kernel \
		   -C gzip -a $(KERNEL_LOAD) -e $(KERNEL_ENTRY) \
		   -n 'Linux-$(KERNELRELEASE)' -d $< $@
endif

$(obj)/uImage: $(obj)/vmlinux.bin.gz FORCE
	$(call if_changed,uimage)
	@echo '  Image $@ is ready'

$(obj)/uvsImage: $(obj)/vmlinux.bin.gz FORCE
	$(call if_changed,uimage_sign)
	@echo '  Image $@ is ready'

$(obj)/vmlinux.bin: vmlinux FORCE
	$(call if_changed,objcopy)

$(obj)/vmlinux.bin.gz: $(obj)/vmlinux.bin FORCE
	$(call if_changed,gzip)

OBJCOPYFLAGS_vmlinux.srec := -I binary -O srec
$(obj)/vmlinux.srec: $(obj)/compressed/vmlinux
	$(call if_changed,objcopy)

OBJCOPYFLAGS_uImage.srec := -I binary -O srec
$(obj)/uImage.srec: $(obj)/uImage
	$(call if_changed,objcopy)

clean-files	+= vmlinux.srec uImage uImage.srec uvsImage
clean-files	+= vmlinux.bin vmlinux.bin.gz
