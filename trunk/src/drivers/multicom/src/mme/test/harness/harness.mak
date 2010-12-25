#
# harness.mak
#
# Copyright (C) STMicroelectronics Limited 2002. All rights reserved.
#
# Test harness include file.
#

include $(RPC_TEST)/harness/target.mak

#
# configurable macros
#

HARNESS_LIB = harness

# ensure we include the harness library
LOCAL_LDFLAGS_ST40_OS21  = -L$(HARNESS_INCLUDES)/$(OBJDIR_ST40_OS21) -lharness -l$(MME_LIBRARY) -l$(EMBXSHM_LIB) -lembxmailbox
LOCAL_LDFLAGS_ST40_LINUX = -L$(HARNESS_INCLUDES)/$(OBJDIR_ST40_LINUX) -lharness -l$(MME_LIBRARY) -lpthread
LOCAL_LDFLAGS_ST40_LINUX_KO = $(HARNESS_INCLUDES)/$(OBJDIR_ST40_LINUX)/kernel/libharness.lo
LOCAL_LDFLAGS_ST40_WINCE = -libpath:$(HARNESS_INCLUDES)/$(OBJDIR_ST40_WINCE) harness.lib $(MME_LIBRARY).lib $(EMBXSHM_LIB).lib embxmailbox.lib
LOCAL_LDFLAGS_ST231_OS21 = -L$(HARNESS_INCLUDES)/$(OBJDIR_ST231_OS21) -l$(HARNESS_LIB) -l$(MME_LIBRARY) -l$(EMBXSHM_LIB) -lembxmailbox

#
# explicit rules
#

ifdef TEST

all: $(MAKE_DIRS) app_master app_slave st40_linux_ramdisk

relink : del_targets all

del_targets :
	$(RM) $(call DOSCMD, $(call APPLICATION_0,$(TEST)) $(call APPLICATION_1,$(TEST))) $(RAMDISK_ST40_LINUX)

run : all $(TEST_PASSED)

ifeq ($(PLATFORM),sim231)
$(TEST_PASSED) : $(call APPLICATION_0,$(TEST)) $(call APPLICATION_1,$(TEST))
	$(SIMRUN_ST231_OS21) \
		CONFIG_FILE $(RPC_ROOT)/src/platform/sim231/mb411_audio.txt \
		-- $(call APPLICATION_1,$(TEST)) \
		++ \
		CONFIG_FILE $(RPC_ROOT)/src/platform/sim231/mb411_video.txt \
		-- $(call APPLICATION_0,$(TEST))
	$(ECHO) $(TEST) > $(TEST_PASSED)
else
ifeq ($(PLATFORM),mb376_reversed)
$(TEST_PASSED) : $(call APPLICATION_0,$(TEST)) $(call APPLICATION_1,$(TEST))
	multirun \
		--launch-delay --verbose \
		--master $(call RTLD_1,$(BOARD_1),$(HTI_1),$(call APPLICATION_1,$(TEST))) \
		--slave  $(call RTLD_0,$(BOARD_0),$(HTI_0),$(call APPLICATION_0,$(TEST)))
	$(ECHO) $(TEST) > $(TEST_PASSED)
else
$(TEST_PASSED) : $(call APPLICATION_0,$(TEST)) $(call APPLICATION_1,$(TEST))
	multirun \
		--launch-delay --verbose \
		--master $(call RTLD_0,$(PLATFORM),$(HTI_0),$(call APPLICATION_0,$(TEST))) \
		--slave  $(call RTLD_1,$(BOARD_1),$(HTI_1),$(call APPLICATION_1,$(TEST)))
	$(ECHO) $(TEST) > $(TEST_PASSED)
endif # mb376_reversed
endif # sim231

app_master : CONF_CFLAGS = -DCONF_MASTER
app_master : BOARD = $(BOARD_0)
app_master : MME_LIBRARY = mme_host
app_master : $(call APPLICATION_0,$(TEST))

app_slave : CONF_CFLAGS = -DCONF_SLAVE
app_slave : BOARD = $(BOARD_1)
ifeq ($(CORE_1),st231)
app_slave : HARNESS_LIB = harness_core1
endif
app_slave : MME_LIBRARY = mme_companion
app_slave : $(call APPLICATION_1,$(TEST))

#
# macroized rules (don't try to understand this unless you have to)
#

$(call APPLICATION_0,$(TEST)) : $(call OBJS_0,$(SRCS_MASTER))
	$(call BUILD_APP_0,$(call APPLICATION_0,$(TEST)),$(call OBJS_0,$(SRCS_MASTER)))

$(call APPLICATION_1,$(TEST)) : $(call OBJS_1,$(SRCS_SLAVE))
	$(call BUILD_APP_1,$(call APPLICATION_1,$(TEST)),$(call OBJS_1,$(SRCS_SLAVE)))

endif # TEST is defined

#
# dependancies 
#

$(call OBJS_0,$(SRCS)) $(call OBJS_1,$(SRCS)) : $(HDRS) makefile

#
# manage distributions
#

ifndef DISABLE_INSTALL
DIST_SRC      =$(wildcard makefile *.c *.h)
endif
include $(RPC_ROOT)/src/mkfiles/install.mak
