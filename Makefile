#
# Makefile for building and installing the kernel and all the modules
# Based on Havana's makefile
#
# Changelog
# Date    Author      Comments
# ------------------------------------------------------------------------------
# 100915  paguilar    Original
# 101018  paguilar    Add support for multiple builds
#

include config.in

ROOTFS=$(ROOTFS_NFS)/$(BOARD)

ARCH=sh
CROSS_COMPILE=sh4-linux-

ifndef BOARD
BOARD=
endif

export PATH:=${PATH}:$(PWD)
export ARCH CROSS_COMPILE
export STG_TOPDIR=$(BUILD_STMFB)
export STMFB_ROOT=$(STMFB)

help:
	@echo ""
	@echo "Generic build targets:"
	@echo "    all             - Build it all"
	@echo "    menuconfig      - Configures kernel build"
	@echo "    xconfig         - Graphically configures kernel build"
	@echo "    kernel          - Build kernel"
	@echo "    modules         - Build all drivers"
	@echo "    modules_install - Install all drivers"
	@echo ""
	@echo "Driver build targets:"
	@echo "    stmfb           - Build Frame Buffer modules"
	@echo "    multicom        - Build multicom modules"
	@echo "    player2         - Build player2 modules"
	@echo "    pti             - Build PTI module"
	@echo "    frontends       - Build frontends modules"
	@echo "    avs             - Build AVS module"
	@echo "    e2proc          - Build e2 proc/ module"
	@echo "    lcd             - Build LCD module"
	@echo "    lpc             - Build LPC module"
	@echo "    fpanel          - Build front panel module"
	@echo "    smartcard       - Build smartcard module"
	@echo "    scart           - Build SCART module"
	@echo "    starci2win      - Build SCART module"
	@echo "    qboxhd_generic  - Build generic module"
	@echo "    qboxhdinfo      - Build info /proc module"
	@echo "    delayer         - Build delayer module"
	@echo "    protocol        - Build protocol module"
	@echo ""
	@echo "Applications build targets:"
	@echo "    enigma2         - Build enigma2"
	@echo ""
	@echo "Clean build targets:"
	@echo "    clean           - Remove all modules except kernel build"
	@echo "    distclean       - Remove the whole build directory"
	@echo ""
	@echo "Examples:"
	@echo "    Build everything for the QBoxHD mini:"
	@echo "        make BOARD=qboxhd_mini all"
	@echo "    Build the kernel for the QBoxHD mini:"
	@echo "        make BOARD=qboxhd_mini kernel"
	@echo "    Build and install player2 drivers for the QBoxHD:"
	@echo "        make BOARD=qboxhd player2"
	@echo "        make BOARD=qboxhd player2_install"
	@echo "    Build the enigma2:"
	@echo "        make BOARD=qboxhd enigma2"
	@echo ""

config.in:
	@echo "Please supply a config.in file"
	@false

all: kernel kernel_install modules modules_install


.check_board:
	$(if $(BOARD),,$(error "FATAL: Invalid board. Type 'make help'"))


###############################################################################
### Kernel
###############################################################################

KERNEL_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

menuconfig: .check_board $(BUILD_KERNEL)
	$(KERNEL_COMMAND) menuconfig

xconfig: $(BUILD_KERNEL)
	$(KERNEL_COMMAND) xconfig

kernel: .check_board config.in $(BUILD_KERNEL) .kernel

.kernel:
	$(KERNEL_COMMAND) vmlinux uImage

kernel_install:
	@echo "Copying kernel image from '$(BUILD_KERNEL)/arch/sh/boot/uImage' to '$(TFTPBOOT)/uImage_$(BOARD)'"
	@cp $(BUILD_KERNEL)/arch/sh/boot/uImage $(TFTPBOOT)/uImage_$(BOARD)


###############################################################################
### Modules 
###############################################################################

STMFB_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_STMFB)/Linux/kbuild ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

MULTICOM_COMMAND=make -C $(BUILD_MULTICOM) ENABLE_ST40_LINUX=1 DISABLE_ST40_OS21=1 KERNELDIR=$(SRC_KERNEL) O=$(BUILD_KERNEL) INSTALL_MOD_PATH=$(ROOTFS) 

PLAYER2_COMMAND=make -C $(SRC_KERNEL)  O=$(BUILD_KERNEL) M=$(BUILD_PLAYER2)/linux ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS) PLAYER2_PATH=$(SRC_PLAYER2) INCLUDE_PATH=$(SRC_INC) STMFB_PATH=$(SRC_STMFB) FE_PATH=$(SRC_FE) PTI_PATH=$(SRC_PTI) AVS_PATH=$(SRC_AVS) DELAYER_PATH=$(SRC_DELAYER) EXTRA_CFLAGS="-Idrivers/media/dvb/dvb-core"

PTI_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_PTI) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS) PLAYER2_PATH=$(SRC_PLAYER2) INCLUDE_PATH=$(SRC_INC) EXTRA_CFLAGS="-Idrivers/media/dvb/dvb-core -DDEBUG_DVB -D__TDT__" 

FE_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_FE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS) PLAYER2_PATH=$(SRC_PLAYER2) INCLUDE_PATH=$(SRC_INC) BOARD_TYPE=$(BOARD) EXTRA_CFLAGS="-Idrivers/media/dvb/dvb-core -DDEBUG_DVB -D__TDT__"

AVS_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_AVS) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

E2PROC_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_E2PROC) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

LCD_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_LCD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

LPC_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_LPC) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

FPANEL_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_FPANEL) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS) BOARD_TYPE=$(BOARD)

SMARTCARD_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_SMARTCARD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

SCART_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_SCART) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS) BOARD_TYPE=$(BOARD)

STARCI2WIN_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_STARCI2WIN) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS) BOARD_TYPE=$(BOARD)

QBOXHDGENERIC_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_QBOXHDGENERIC) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

QBOXHDINFO_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_QBOXHDINFO) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)

DELAYER_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_DELAYER) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS) PLAYER2_PATH=$(SRC_PLAYER2) INCLUDE_PATH=$(SRC_INC) EXTRA_CFLAGS="-Idrivers/media/dvb/dvb-core"

PROTOCOL_COMMAND=make -C $(SRC_KERNEL) O=$(BUILD_KERNEL) M=$(BUILD_PROTOCOL) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) INSTALL_MOD_PATH=$(ROOTFS)




modules: .check_board config.in .modules_kernel stmfb multicom player2 pti frontends avs e2proc lcd lpc fpanel smartcard scart starci2win qboxhd_generic qboxhdinfo delayer protocol
	@echo All modules built

.modules_kernel: $(BUILD_KERNEL)/.config
	$(KERNEL_COMMAND) modules
	@touch $@

stmfb: .check_board config.in $(BUILD_STMFB)
	$(STMFB_COMMAND) modules

multicom: .check_board config.in $(BUILD_MULTICOM)
	$(MULTICOM_COMMAND) modules

player2: .check_board config.in $(BUILD_PLAYER2)
	$(PLAYER2_COMMAND) modules

pti: .check_board config.in $(BUILD_PTI)
	$(PTI_COMMAND) modules

frontends: .check_board config.in $(BUILD_FE)
	$(FE_COMMAND) modules

avs: .check_board config.in $(BUILD_AVS)
	$(AVS_COMMAND) modules

e2proc: .check_board config.in $(BUILD_E2PROC)
	$(E2PROC_COMMAND) modules

lcd: .check_board config.in $(BUILD_LCD)
	$(LCD_COMMAND) modules

lpc: .check_board config.in $(BUILD_LPC)
	$(LPC_COMMAND) modules

fpanel: .check_board config.in $(BUILD_FPANEL)
	$(FPANEL_COMMAND) modules

smartcard: .check_board config.in $(BUILD_SMARTCARD)
	$(SMARTCARD_COMMAND) modules

scart: .check_board config.in $(BUILD_SCART)
	$(SCART_COMMAND) modules

starci2win: .check_board config.in $(BUILD_STARCI2WIN)
ifeq ($(BOARD),qboxhd)
	$(STARCI2WIN_COMMAND) modules
endif

qboxhd_generic: .check_board config.in $(BUILD_QBOXHDGENERIC)
	$(QBOXHDGENERIC_COMMAND) modules

qboxhdinfo: .check_board config.in $(BUILD_QBOXHDINFO)
	$(QBOXHDINFO_COMMAND) modules

delayer: .check_board config.in $(BUILD_DELAYER)
	$(DELAYER_COMMAND) modules

protocol: .check_board config.in $(BUILD_PROTOCOL)
	$(PROTOCOL_COMMAND) modules





###
### Modules installation
###

modules_install: .check_board .modules_install_kernel stmfb_install multicom_install player2_install pti_install frontends_install avs_install e2proc_install lcd_install lpc_install fpanel_install smartcard_install scart_install qboxhd_generic_install qboxhdinfo_install starci2win_install delayer_install protocol_install
	@echo All modules installed

.modules_install_kernel: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
.modules_install_kernel:
	@echo Installing kernel modules
	@cp -v `find $(BUILD_KERNEL) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
#	$(KERNEL_COMMAND) modules_install

stmfb_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
stmfb_install: config.in
	@echo Installing stmfb modules
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_STMFB) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)

multicom_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
multicom_install: config.in
	@echo Installing multicom modules
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_MULTICOM) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)

player2_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
player2_install: config.in
	@echo Installing player2 modules
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_PLAYER2) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(PLAYER2_COMMAND) modules_install

pti_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
pti_install: config.in
	@echo Installing pti module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_PTI) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(PTI_COMMAND) modules_install

frontends_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
frontends_install: config.in
	@echo Installing frontends modules
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_FE) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(FE_COMMAND) modules_install

avs_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
avs_install: config.in
	@echo Installing frontends module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_AVS) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(AVS_COMMAND) modules_install

e2proc_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
e2proc_install: config.in
	@echo Installing e2proc module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_E2PROC) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(E2PROC_COMMAND) modules_install

lcd_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
lcd_install: config.in
	@echo Installing lcd module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_LCD) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(LCD_COMMAND) modules_install

lpc_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
lpc_install: config.in
	@echo Installing lpc module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_LPC) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(LPC_COMMAND) modules_install

fpanel_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
fpanel_install: config.in
	@echo Installing front panel module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_FPANEL) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(FPANEL_COMMAND) modules_install

smartcard_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
smartcard_install: config.in
	@echo Installing smartcard module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_SMARTCARD) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(SMARTCARD_COMMAND) modules_install

scart_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
scart_install: config.in
	@echo Installing scart module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_SCART) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(SCART_COMMAND) modules_install

starci2win_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
starci2win_install: config.in
	@echo Installing starci2win module
	depmod -b $(ROOTFS) -v $(TAG)
ifeq ($(BOARD),qboxhd)
	@cp -v `find $(BUILD_STARCI2WIN) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(STARCI2WIN_COMMAND) modules_install
endif

qboxhd_generic_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
qboxhd_generic_install: config.in
	@echo Installing generic module
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_QBOXHDGENERIC) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(QBOXHDGENERIC_COMMAND) modules_install

qboxhdinfo_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
qboxhdinfo_install: config.in
	@echo Installing qboxhd info modules
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_QBOXHDINFO) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(QBOXHDINFO_COMMAND) modules_install

delayer_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
delayer_install: config.in
	@echo Installing delayer modules
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_DELAYER) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(DELAYER_COMMAND) modules_install

protocol_install: TAG = $(shell cat $(BUILD_KERNEL)/include/linux/utsrelease.h | cut -d'"' -f2)
protocol_install: config.in
	@echo Installing qboxhd info modules
	depmod -b $(ROOTFS) -v $(TAG)
	@cp -v `find $(BUILD_PROTOCOL) -name *.ko` $(ROOTFS)/lib/modules/$(TAG)
	$(PROTOCOL_COMMAND) modules_install


###############################################################################
### Kernel and modules clean rules
###############################################################################

modules_clean: .check_board .modules_clean_kernel stmfb_clean multicom_clean player2_clean pti_clean frontends_clean avs_clean e2proc_clean lcd_clean lpc_clean fpanel_clean smartcard_clean scart_clean qboxhd_generic_clean qboxhdinfo_clean starci2win_clean delayer_clean protocol_clean 

stmfb_clean:
	@rm -rf $(BUILD_STMFB)

multicom_clean:
	@rm -rf $(BUILD_MULTICOM)

player2_clean:
	@rm -rf $(BUILD_PLAYER2)

pti_clean:
	@rm -rf $(BUILD_PTI)

frontends_clean:
	@rm -rf $(BUILD_FE)

avs_clean:
	@rm -rf $(BUILD_AVS)

e2_proc_clean:
	@rm -rf $(BUILD_E2PROC)

lcd_clean:
	@rm -rf $(BUILD_LCD)

lpc_clean:
	@rm -rf $(BUILD_LPC)

fpanel_clean:
	@rm -rf $(BUILD_FPANEL)

smartcard_clean:
	@rm -rf $(BUILD_SMARTCARD)

scart_clean:
	@rm -rf $(BUILD_SCART)

qboxhd_generic_clean:
	@rm -rf $(BUILD_QBOXHDGENERIC)

qboxhdinfo_clean:
	@rm -rf $(BUILD_QBOXHDINFO)

starci2win_clean:
	@rm -rf $(BUILD_STARCI2WIN)

delayer_clean:
	@rm -rf $(BUILD_DELAYER)

protocol_clean:
	@rm -rf $(BUILD_PROTOCOL)


###############################################################################
### Kernel and modules build rules
###############################################################################

$(BUILD_KERNEL): $(SRC_KERNEL)/config.default
	@echo Creating kernel build directory and applying default configuration
	@mkdir -p $(BUILD_KERNEL)
	@echo "Applying default configuration settings (check log incase of errors)"
	@cp $(SRC_KERNEL)/config.default $(BUILD_KERNEL)/.config
	@$(KERNEL_COMMAND) oldconfig prepare scripts maketools
	@touch $@

$(BUILD_STMFB): .check_board config.in $(SRC_STMFB) $(BUILD_DIR) .FORCE
	@echo Building stmfb
	@-mkdir -p $(BUILD_STMFB)
	@find $(BUILD_STMFB) -type l -exec rm {} \;
	@cp -prs $(SRC_STMFB)/* $(BUILD_STMFB)

$(BUILD_MULTICOM): .check_board config.in $(SRC_MULTICOM) $(BUILD_DIR) .FORCE
	@echo Building multicom
	@-mkdir -p $(BUILD_MULTICOM)
	@-rm -f $(BUILD_MULTICOM)/include/embxshm.h
	@find $(BUILD_MULTICOM) -type l -exec rm {} \;
	@-(cd $(BUILD_MULTICOM); rm -f `(cd $(SRC_MULTICOM); ls lib/linux/st40/lib*.so)`) 
	@cp -prs $(SRC_MULTICOM)/* $(BUILD_MULTICOM)

$(BUILD_PLAYER2): .check_board config.in $(SRC_PLAYER2) $(BUILD_DIR) .FORCE
	@echo Building player2 
	@-mkdir -p $(BUILD_PLAYER2)
	@find $(BUILD_PLAYER2) -type l -exec rm {} \;
	@cp -prs $(SRC_PLAYER2)/* $(BUILD_PLAYER2)

$(BUILD_PTI): .check_board config.in $(SRC_PTI) $(BUILD_DIR) .FORCE
	@echo Building pti 
	@-mkdir -p $(BUILD_PTI)
	@find $(BUILD_PTI) -type l -exec rm {} \;
	@cp -prs $(SRC_PTI)/* $(BUILD_PTI)

$(BUILD_FE): .check_board config.in $(SRC_FE) $(BUILD_DIR) .FORCE
	@echo Building frontends 
	@-mkdir -p $(BUILD_FE)
	@find $(BUILD_FE) -type l -exec rm {} \;
	@cp -prs $(SRC_FE)/* $(BUILD_FE)

$(BUILD_AVS): .check_board config.in $(SRC_AVS) $(BUILD_DIR) .FORCE
	@echo Building avs 
	@-mkdir -p $(BUILD_AVS)
	@find $(BUILD_AVS) -type l -exec rm {} \;
	@cp -prs $(SRC_AVS)/* $(BUILD_AVS)

$(BUILD_E2PROC): .check_board config.in $(SRC_E2PROC) $(BUILD_DIR) .FORCE
	@echo Building e2_proc 
	@-mkdir -p $(BUILD_E2PROC)
	@find $(BUILD_E2PROC) -type l -exec rm {} \;
	@cp -prs $(SRC_E2PROC)/* $(BUILD_E2PROC)

$(BUILD_LCD): .check_board config.in $(SRC_LCD) $(BUILD_DIR) .FORCE
	@echo Building lcd 
	@-mkdir -p $(BUILD_LCD)
	@find $(BUILD_LCD) -type l -exec rm {} \;
	@cp -prs $(SRC_LCD)/* $(BUILD_LCD)

$(BUILD_LPC): .check_board config.in $(SRC_LPC) $(BUILD_DIR) .FORCE
	@echo Building lpc 
	@-mkdir -p $(BUILD_LPC)
	@find $(BUILD_LPC) -type l -exec rm {} \;
	@cp -prs $(SRC_LPC)/* $(BUILD_LPC)

$(BUILD_FPANEL): .check_board config.in $(SRC_FPANEL) $(BUILD_DIR) .FORCE
	@echo Building front panel 
	@-mkdir -p $(BUILD_FPANEL)
	@find $(BUILD_FPANEL) -type l -exec rm {} \;
	@cp -prs $(SRC_FPANEL)/* $(BUILD_FPANEL)

$(BUILD_SMARTCARD): .check_board config.in $(SRC_SMARTCARD) $(BUILD_DIR) .FORCE
	@echo Building smartcard 
	@-mkdir -p $(BUILD_SMARTCARD)
	@find $(BUILD_SMARTCARD) -type l -exec rm {} \;
	@cp -prs $(SRC_SMARTCARD)/* $(BUILD_SMARTCARD)

$(BUILD_SCART): .check_board config.in $(SRC_SCART) $(BUILD_DIR) .FORCE
	@echo Building scart 
	@-mkdir -p $(BUILD_SCART)
	@find $(BUILD_SCART) -type l -exec rm {} \;
	@cp -prs $(SRC_SCART)/* $(BUILD_SCART)

$(BUILD_STARCI2WIN): .check_board config.in $(SRC_STARCI2WIN) $(BUILD_DIR) .FORCE
ifeq ($(BOARD),qboxhd)
	@echo Building starci2win 
	@-mkdir -p $(BUILD_STARCI2WIN)
	@find $(BUILD_STARCI2WIN) -type l -exec rm {} \;
	@cp -prs $(SRC_STARCI2WIN)/* $(BUILD_STARCI2WIN)
endif

$(BUILD_QBOXHDGENERIC): .check_board config.in $(SRC_QBOXHDGENERIC) $(BUILD_DIR) .FORCE
	@echo Building qboxhd generic 
	@-mkdir -p $(BUILD_QBOXHDGENERIC)
	@find $(BUILD_QBOXHDGENERIC) -type l -exec rm {} \;
	@cp -prs $(SRC_QBOXHDGENERIC)/* $(BUILD_QBOXHDGENERIC)

$(BUILD_QBOXHDINFO): .check_board config.in $(SRC_QBOXHDINFO) $(BUILD_DIR) .FORCE
	@echo Building qboxhd info
	@-mkdir -p $(BUILD_QBOXHDINFO)
	@find $(BUILD_QBOXHDINFO) -type l -exec rm {} \;
	@cp -prs $(SRC_QBOXHDINFO)/* $(BUILD_QBOXHDINFO)

$(BUILD_DELAYER): .check_board config.in $(SRC_DELAYER) $(BUILD_DIR) .FORCE
	@echo Building delayer
	@-mkdir -p $(BUILD_DELAYER)
	@find $(BUILD_DELAYER) -type l -exec rm {} \;
	@cp -prs $(SRC_DELAYER)/* $(BUILD_DELAYER)

$(BUILD_PROTOCOL): .check_board config.in $(SRC_PROTOCOL) $(BUILD_DIR) .FORCE
	@echo Building protocol
	@-mkdir -p $(BUILD_PROTOCOL)
	@find $(BUILD_PROTOCOL) -type l -exec rm {} \;
	@cp -prs $(SRC_PROTOCOL)/* $(BUILD_PROTOCOL)



###############################################################################
### Enigma2
###############################################################################

enigma2: .check_board
	@perl sdk/e2.pl build $(BOARD)

enigma2_install: .check_board
	@perl sdk/e2.pl install $(BOARD)

enigma2_clean: .check_board
	@perl sdk/e2.pl clean $(BOARD)

enigma2_distclean: .check_board
	@perl sdk/e2.pl distclean $(BOARD)


###############################################################################
### Clean rules
###############################################################################

.FORCE:

clean:
	@echo Removing modules
	@-rm -rf .modules* $(BUILD_STMFB) $(BUILD_MULTICOM) $(BUILD_PLAYER2) $(BUILD_PTI) $(BUILD_FE) $(BUILD_AVS) $(BUILD_E2PROC) $(BUILD_LCD) $(BUILD_LPC) $(BUILD_FPANEL) $(BUILD_SMARTCARD) $(BUILD_SCART) $(BUILD_STARCI2WIN) $(BUILD_QBOXHDGENERIC) $(BUILD_QBOXHDINFO) $(BUILD_DELAYER) $(BUILD_PROTOCOL)

distclean:
	@echo Removing building directory $(BUILD_DIR)
	@-rm -rf $(BUILD_DIR)

