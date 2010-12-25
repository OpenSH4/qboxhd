#
# harness.mak
#
# Copyright (C) STMicroelectronics Limited 2003. All rights reserved.
#
# Build system interface for the RPC examples.
#

ENABLE_COMENTARY=1	# Allow people to see how these build without
			# having to use the EMBX/RPC make system

RPC_ROOT = ../../..

# select the board/bsp/cpu etc.
include $(RPC_ROOT)/src/mkfiles/host.mak
include $(RPC_ROOT)/src/mkfiles/platform.mak

#
# configurable macros
#

# these 'pretty' macros are used in commentaries
MACHINE_0 = $(call TOUPPER,$(CORE_0)/$(OS_0))
MACHINE_1 = $(call TOUPPER,$(CORE_1)/$(OS_1))

# these nearly identical 'functional' macros are used to macroize
# variable expansion [e.g. $(CPP_$(MASTER))]
MASTER = $(call TOUPPER,$(CORE_0)_$(OS_0))
SLAVE  = $(call TOUPPER,$(CORE_1)_$(OS_1))

# prevent non-rpc examples from issuing warning about redefined rules
ifndef STUB_MASTER
STUB_MASTER=mstr
STUB_SLAVE=slve
endif

LOCAL_CFLAGS += -I../../harness

# default to the companion library (this will be overridden when build hosts)
MME_LIBRARY = mme_companion

LOCAL_LDFLAGS_ST40_OS21 = -l$(MME_LIBRARY) -lembxmailbox -lembxshm
LOCAL_LDFLAGS_ST40_LINUX = -l$(MME_LIBRARY) -lpthread
LOCAL_LDFLAGS_ST40_WINCE = $(MME_LIBRARY).lib embxmailbox.lib embxshm.lib embxshell.lib
LOCAL_LDFLAGS_ST231_OS21 = -l$(MME_LIBRARY) -lembxmailbox -lembxshm

#
# explicit rules
#

example: $(MAKE_DIRS) app_master app_slave

app_master : BOARD = $(BOARD_0)
app_master : MME_LIBRARY = mme_host
app_master : $(call APPLICATION_0,$(APP_MASTER))

app_slave  : BOARD = $(BOARD_1)
app_slave  : $(call APPLICATION_1,$(APP_SLAVE))

ifdef ENABLE_EXAMPLES
all : example
else
all :
	@$(ECHO) Please use \"make example\" to build this directory
endif

clean :
	-$(RM) $(call DOSCMD,$(CLEAN_DIRS) *.stubs.c *.stubs.cpped)

#
# build rules
#
# Note: 
# The build rules in this build harness are all inherited from platform.mak. Their
# complexity is inherent in the number of processor combinations supported by the
# Multicom build/example harness. This should not under any circumstances be
# considered a tutorial for writing RPC build systems, the build commentary is
# provided so you can identify the purpose of each of the commands the build system
# issues.
#

$(call APPLICATION_0,$(APP_MASTER)) : $(call OBJS_0,$(SRCS_MASTER))
	@$(ECHO) +++ Link $(MACHINE_0) application for CPU 0 [$(call APPLICATION_0,$(APP_MASTER))]. +++
	$(call BUILD_APP_0,$(call APPLICATION_0,$(APP_MASTER)),$(call OBJS_0,$(SRCS_MASTER)))

$(call APPLICATION_1,$(APP_SLAVE)) : $(call OBJS_1,$(SRCS_SLAVE))
	@$(ECHO) +++ Link $(MACHINE_1) application for CPU 1 [$(call APPLICATION_1,$(APP_SLAVE))]. +++
	$(call BUILD_APP_1,$(call APPLICATION_1,$(APP_SLAVE)),$(call OBJS_1,$(SRCS_SLAVE)))


$(STUB_MASTER).stubs.cpped : $(SRC_IDL)
	@$(ECHO) +++ C pre-process the interface definition for $(STUB_MASTER). +++
	$(CPP_$(MASTER)) $(CFLAGS_$(MASTER)) $< > $@

$(STUB_SLAVE).stubs.cpped : $(SRC_IDL) 
	@$(ECHO) +++ C pre-process the interface definition for $(STUB_SLAVE). +++
	$(CPP_$(SLAVE)) $(CFLAGS_$(SLAVE)) $< > $@

$(STUB_MASTER).stubs.c : $(STUB_MASTER).stubs.cpped
	@$(ECHO) +++ Generate the $(STUB_MASTER) stubs [$@]. +++
	strpcgen $(STRPCGEN_FLAGS) -i $< -o $@ -a $(STUB_MASTER)

$(STUB_SLAVE).stubs.c : $(STUB_SLAVE).stubs.cpped
	@$(ECHO) +++ Generate the $(STUB_SLAVE) stubs [$@]. +++
	strpcgen $(STRPCGEN_FLAGS) -i $< -o $@ -a $(STUB_SLAVE)

#
# manage distributions
#

DIST_SRC = $(wildcard *.c *.h makefile) README
include $(RPC_ROOT)/src/mkfiles/install.mak
