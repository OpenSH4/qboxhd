#
# mkfiles/install.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Rules to make an RPC distribution
#

ifndef RPC_DIST
RPC_DIST := $(RPC_ROOT)
endif


ifeq (1,$(DISABLE_SRC_INSTALL))
DIST_SRC_INSTALL_TARGETS=
else
DIST_SRC_INSTALL_TARGETS= dist_src
endif


ifeq (1,$(DISABLE_PUBLIC_INSTALL))
DIST_PUBLIC_INSTALL_TARGETS=
else
DIST_PUBLIC_INSTALL_TARGETS= \
	$(DIST_LOCAL_INSTALL_TARGET) \
	dist_include                 \
	dist_st40_os21_lib           \
	dist_st40_linux_lib          \
	dist_st40_linux_bin          \
	dist_st40_wince_lib          \
	dist_st40_wince_bin          \
	dist_st231_os21_lib          \
	dist_st231_linux_lib         \
	dist_st231_linux_bin         \
	dist_sparc_solaris_lib       \
	dist_sparc_solaris_bin       \
	dist_ia32_winnt_lib          \
	dist_ia32_winnt_bin          \
	dist_ia32_linux_lib          \
	dist_ia32_linux_bin 

endif

install : all                            \
	  $(DIST_SRC_INSTALL_TARGETS)    \
	  $(DIST_PUBLIC_INSTALL_TARGETS)


# Source targets are only created if DIST_SRC_LOCATION
# is defined and there is source to distrbute.
# DIST_SRC_LOCATION is only defined if the make
# has been started from the top level of the tree.
# This allows install to be used during development
# to install the libraries and headers in the development
# tree, from any directory. Source will only get
# copied when creating a complete distribution tree.
ifdef DIST_SRC_LOCATION

ifneq (,$(DIST_SRC))
DIR_SRC  := $(RPC_DIST)/$(DIST_SRC_LOCATION)
DIST_SRC := $(foreach f,$(DIST_SRC),$(DIR_SRC)/$f)

$(DIR_SRC)/% : %
	$(CP) $(call DOSCMD,$< $@)

$(DIR_SRC):
	-$(MKDIR) $(call DOSCMD,$(DIR_SRC))

endif

endif

dist_src : $(DIR_SRC) $(DIST_SRC)


# The rest of the rules follow the same form as below.
# Each set of rules create and populate a specific 
# subdirectory in the installion directories include, bin and lib.
# We provide a set of rules for a range of chip and operating
# system combinations that may not all be used right now.
# The rules only create those directories in the installation
# when they are actually needed. 
ifneq (,$(DIST_INCLUDE))

DIR_INCLUDE  := $(RPC_DIST)/include
DIST_INCLUDE := $(foreach f,$(DIST_INCLUDE),$(DIR_INCLUDE)/$f)

$(DIR_INCLUDE)/% : %
	$(CP) $(call DOSCMD,$< $@)

$(DIR_INCLUDE):
	-$(MKDIR) $(call DOSCMD,$(DIR_INCLUDE))

endif

dist_include : $(DIR_INCLUDE) $(DIST_INCLUDE)



ifneq (,$(DIST_ST231_OS21_LIB))

DIR_ST231_OS21_LIB  := $(RPC_DIST)/lib/os21/st231
DIST_ST231_OS21_LIB := $(foreach f,$(DIST_ST231_OS21_LIB),$(DIR_ST231_OS21_LIB)/$f)

$(DIR_ST231_OS21_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST231_OS21_LIB))

$(DIR_ST231_OS21_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST231_OS21_LIB))

endif

dist_st231_os21_lib : $(DIR_ST231_OS21_LIB) $(DIST_ST231_OS21_LIB)



ifneq (,$(DIST_ST231_LINUX_LIB))

DIR_ST231_LINUX_LIB  := $(RPC_DIST)/lib/linux/st231
DIST_ST231_LINUX_LIB := $(foreach f,$(DIST_ST231_LINUX_LIB),$(DIR_ST231_LINUX_LIB)/$f)

$(DIR_ST231_LINUX_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST231_LINUX_LIB))

$(DIR_ST231_LINUX_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST231_LINUX_LIB))

endif

dist_st231_linux_lib : $(DIR_ST231_LINUX_LIB) $(DIST_ST231_LINUX_LIB)



ifneq (,$(DIST_ST231_LINUX_BIN))

DIR_ST231_LINUX_BIN  := $(RPC_DIST)/bin/linux/st231
DIST_ST231_LINUX_BIN := $(foreach f,$(DIST_ST231_LINUX_BIN),$(DIR_ST231_LINUX_BIN)/$f)

$(DIR_ST231_LINUX_BIN)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST231_LINUX_BIN))

$(DIR_ST231_LINUX_BIN):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST231_LINUX_BIN))

endif

dist_st231_linux_bin : $(DIR_ST231_LINUX_BIN) $(DIST_ST231_LINUX_BIN)



ifneq (,$(DIST_ST40_OS21_LIB))

DIR_ST40_OS21_LIB  := $(RPC_DIST)/lib/os21/st40
DIST_ST40_OS21_LIB := $(foreach f,$(DIST_ST40_OS21_LIB),$(DIR_ST40_OS21_LIB)/$f)

$(DIR_ST40_OS21_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST40_OS21_LIB))

$(DIR_ST40_OS21_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST40_OS21_LIB))

endif

dist_st40_os21_lib : $(DIR_ST40_OS21_LIB) $(DIST_ST40_OS21_LIB)



ifneq (,$(DIST_ST40_LINUX_LIB))

DIR_ST40_LINUX_LIB  := $(RPC_DIST)/lib/linux/st40
DIST_ST40_LINUX_LIB := $(foreach f,$(DIST_ST40_LINUX_LIB),$(DIR_ST40_LINUX_LIB)/$f)

$(DIR_ST40_LINUX_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST40_LINUX_LIB))

$(DIR_ST40_LINUX_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST40_LINUX_LIB))

endif

dist_st40_linux_lib : $(DIR_ST40_LINUX_LIB) $(DIST_ST40_LINUX_LIB)



ifneq (,$(DIST_ST40_LINUX_BIN))

DIR_ST40_LINUX_BIN  := $(RPC_DIST)/bin/linux/st40
DIST_ST40_LINUX_BIN := $(foreach f,$(DIST_ST40_LINUX_BIN),$(DIR_ST40_LINUX_BIN)/$f)

$(DIR_ST40_LINUX_BIN)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST40_LINUX_BIN))

$(DIR_ST40_LINUX_BIN):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST40_LINUX_BIN))

endif

dist_st40_linux_bin : $(DIR_ST40_LINUX_BIN) $(DIST_ST40_LINUX_BIN)



ifneq (,$(DIST_ST40_WINCE_LIB))

DIR_ST40_WINCE_LIB  := $(RPC_DIST)/lib/wince/st40
DIST_ST40_WINCE_LIB := $(foreach f,$(DIST_ST40_WINCE_LIB),$(DIR_ST40_WINCE_LIB)/$f)

$(DIR_ST40_WINCE_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST40_WINCE_LIB))

$(DIR_ST40_WINCE_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST40_WINCE_LIB))

endif

dist_st40_wince_lib : $(DIR_ST40_WINCE_LIB) $(DIST_ST40_WINCE_LIB)



ifneq (,$(DIST_ST40_WINCE_BIN))

DIR_ST40_WINCE_BIN  := $(RPC_DIST)/bin/wince/st40
DIST_ST40_WINCE_BIN := $(foreach f,$(DIST_ST40_WINCE_BIN),$(DIR_ST40_WINCE_BIN)/$f)

$(DIR_ST40_WINCE_BIN)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_ST40_WINCE_BIN))

$(DIR_ST40_WINCE_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_ST40_WINCE_LIB))

endif

dist_st40_wince_bin : $(DIR_ST40_WINCE_BIN) $(DIST_ST40_WINCE_BIN)



ifneq (,$(DIST_SPARC_SOLARIS_LIB))

DIR_SPARC_SOLARIS_LIB  := $(RPC_DIST)/lib/solaris/sparc
DIST_SPARC_SOLARIS_LIB := $(foreach f,$(DIST_SPARC_SOLARIS_LIB),$(DIR_SPARC_SOLARIS_LIB)/$f)

$(DIR_SPARC_SOLARIS_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_SPARC_SOLARIS_LIB))

$(DIR_SPARC_SOLARIS_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_SPARC_SOLARIS_LIB))

endif

dist_sparc_solaris_lib : $(DIR_SPARC_SOLARIS_LIB) $(DIST_SPARC_SOLARIS_LIB)



ifneq (,$(DIST_SPARC_SOLARIS_BIN))

DIR_SPARC_SOLARIS_BIN  := $(RPC_DIST)/bin/solaris/sparc
DIST_SPARC_SOLARIS_BIN := $(foreach f,$(DIST_SPARC_SOLARIS_BIN),$(DIR_SPARC_SOLARIS_BIN)/$f)

$(DIR_SPARC_SOLARIS_BIN)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_SPARC_SOLARIS_BIN))

$(DIR_SPARC_SOLARIS_BIN):
	-$(MKDIR) $(call DOSCMD,$(DIR_SPARC_SOLARIS_BIN))

endif

dist_sparc_solaris_bin : $(DIR_SPARC_SOLARIS_BIN) $(DIST_SPARC_SOLARIS_BIN)



ifneq (,$(DIST_IA32_WINNT_LIB))

DIR_IA32_WINNT_LIB  := $(RPC_DIST)/lib/winnt/ia32
DIST_IA32_WINNT_LIB := $(foreach f,$(DIST_IA32_WINNT_LIB),$(DIR_IA32_WINNT_LIB)/$f)

$(DIR_IA32_WINNT_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_IA32_WINNT_LIB))

$(DIR_IA32_WINNT_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_IA32_WINNT_LIB))

endif

dist_ia32_winnt_lib : $(DIR_IA32_WINNT_LIB) $(DIST_IA32_WINNT_LIB)



ifneq (,$(DIST_IA32_WINNT_BIN))

DIR_IA32_WINNT_BIN  := $(RPC_DIST)/bin/winnt/ia32
DIST_IA32_WINNT_BIN := $(foreach f,$(DIST_IA32_WINNT_BIN),$(DIR_IA32_WINNT_BIN)/$f)

$(DIR_IA32_WINNT_BIN)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_IA32_WINNT_BIN))

$(DIR_IA32_WINNT_BIN):
	-$(MKDIR) $(call DOSCMD,$(DIR_IA32_WINNT_BIN))

endif

dist_ia32_winnt_bin : $(DIR_IA32_WINNT_BIN) $(DIST_IA32_WINNT_BIN)



ifneq (,$(DIST_IA32_LINUX_LIB))

DIR_IA32_LINUX_LIB  := $(RPC_DIST)/lib/linux/ia32
DIST_IA32_LINUX_LIB := $(foreach f,$(DIST_IA32_LINUX_LIB),$(DIR_IA32_LINUX_LIB)/$f)

$(DIR_IA32_LINUX_LIB)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_IA32_LINUX_LIB))

$(DIR_IA32_LINUX_LIB):
	-$(MKDIR) $(call DOSCMD,$(DIR_IA32_LINUX_LIB))

endif

dist_ia32_linux_lib : $(DIR_IA32_LINUX_LIB) $(DIST_IA32_LINUX_LIB)



ifneq (,$(DIST_IA32_LINUX_BIN))

DIR_IA32_LINUX_BIN  := $(RPC_DIST)/bin/linux/ia32
DIST_IA32_LINUX_BIN := $(foreach f,$(DIST_IA32_LINUX_BIN),$(DIR_IA32_LINUX_BIN)/$f)

$(DIR_IA32_LINUX_BIN)/% : %
	$(CP) $(call DOSCMD,$< $(DIR_IA32_LINUX_BIN))

$(DIR_IA32_LINUX_BIN):
	-$(MKDIR) $(call DOSCMD,$(DIR_IA32_LINUX_BIN))

endif

dist_ia32_linux_bin : $(DIR_IA32_LINUX_BIN) $(DIST_IA32_LINUX_BIN)



