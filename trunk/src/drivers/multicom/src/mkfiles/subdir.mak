#
# mkfiles/rules.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# Rules to build subdirectories
#

# if ACTION is set to stop then suppress the dive
ifeq ($(ACTION),stop)
SUBDIRS =
endif

ifeq (1, $(DISABLE_SRC_INSTALL))
LOCAL_MAKEFLAGS += DISABLE_SRC_INSTALL=1
endif

ifeq (1, $(DISABLE_PUBLIC_INSTALL))
LOCAL_MAKEFLAGS += DISABLE_PUBLIC_INSTALL=1
endif

all : ACTION = all
all : $(SUBDIRS)

clean : ACTION = clean
clean : $(SUBDIRS) $(CLEANDIRS)

install : ACTION = install
install : $(SUBDIRS)

run : ACTION = run
run : $(SUBDIRS)

example : ACTION = example
example : $(SUBDIRS)

ifdef EXTRADIVE
$(EXTRADIVE) : ACTION = $(EXTRADIVE)
$(EXTRADIVE) : $(SUBDIRS)
endif

.PHONY : $(SUBDIRS) $(CLEANDIRS)
$(SUBDIRS) $(CLEANDIRS):
ifdef DIST_SRC_LOCATION
	$(MAKE) -C $@ $(ACTION) $(LOCAL_MAKEFLAGS) DIST_SRC_LOCATION=$(DIST_SRC_LOCATION)/$@
else
	$(MAKE) -C $@ $(ACTION) $(LOCAL_MAKEFLAGS)
endif
