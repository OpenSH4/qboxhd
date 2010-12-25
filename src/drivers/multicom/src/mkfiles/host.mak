#
# mkfiles/host.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001, 2004
#
# Determine the host that the make has been kicked off on and set up for
#   that host
#

all clean install run example $(EXTRADIVE): 

# Clean important variables from the environment
HOSTTYPE =
MAKE_DIRS =

# Figure out what the path seperator is used on this environment.
# If SHELL does not end in .exe we know we are on a Unix and the
# path seperator is :. If SHELL does end in .exe we examine PATH
# we will assume that if it contains a ; then this is the path 
# seperator otherwise we will use :. This can be overridden from
# the command line be explicitly setting PATHSEP.
ifeq (,$(findstring .exe,$(SHELL)))
  PATHSEP = :
else
  ifeq (,$(findstring ;,$(PATH)))
    PATHSEP = :
  else
    PATHSEP = ;
  endif
endif

# Define EXECPATH to be PATH with the seperators converted to spaces.
# This is needed so we can enumerate EXECPATH using foreach. During
# the process we convert any legitimate spaces to __SPACE__ as a means
# to preserve them (the transform will be undone after EXECPATH has
# been enumerated.
EXECPATH := $(subst $(PATHSEP), ,$(subst $(subst _, ,_),__SPACE__,$(PATH)))

# Define FINDEXEC as a macro that allows the PATH (via EXECPATH) to
# be searched for a particular filename. Note that even on Windows
# wildcard is case sensitive so we support multiple arguments to allow
# alternative casings to be provided.
FINDEXEC = $(strip $(foreach d,$(EXECPATH),$(foreach t,$1 $2 $3 $4,$(wildcard $(subst __SPACE__,\ ,$d)/$t))))

# Define TOUPPER, a macro that converts its single argument to upper
# case characters. Do not alter the peculiar line breaks. Make
# interprets the line breaks as the space character so the line breaks
# occur when a space is semantically correct.
TOUPPER=$(subst\
a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst\
g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst\
m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst\
s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst\
y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

# Figure out if we are running on a Unix or PC host.
# If the value of $SHELL includes the sequence sh.exe then assume we are
# building on a Windows host (either native or cygwin) otherwise we will 
# assume some form of Unix host.
ifeq (,$(strip $(HOSTTYPE)))
  ifneq (,$(findstring .exe,$(SHELL)))
    # determine if we are running under Cygwin
    SH_EXE = $(call FINDEXEC,sh.exe,SH.EXE)
    ifeq (,$(SH_EXE))
      HOSTTYPE=WinNT
    else
      HOSTTYPE=WinNT_cygwin
      # switch the slashes in RPC_ROOT / (because \ is the shell escape character)
      RPC_ROOT:=$(subst \,/,$(RPC_ROOT))
    endif
  else

    # determine whether we are on Linux or Solaris
    ifneq (,$(findstring Linux,$(shell uname)))
      HOSTTYPE=Linux

      # Don't allow builds on an SH4 target, only ix86 and now x86_64
      ifeq (,$(filter i%86 x86_64,$(shell uname -m)))
        $(error Building using Linux is only supported on ix86 platforms)
      endif
    else
      HOSTTYPE=Solaris
    endif

  endif
endif

# setup file handling
ifeq ($(HOSTTYPE),WinNT)
  CP         = copy
  DATE       = date /t
  ECHO       = echo
  MKDIR      = mkdir
  PWD        = cd
  RM         = del /f /q
  RMTREE     = rmdir /s /q
  DOSCMD     = $(subst /,\,$1)
else
  CP         = cp -f
  DATE       = date
  ECHO       = echo
  MKDIR      = mkdir -p
  PWD        = pwd
  RM         = rm -f
  RMTREE     = rm -rf
  DOSCMD     = $1
endif

ifeq ($(PERL),)
  PERL       = perl
endif

ifneq (,$(wildcard $(RPC_ROOT)/src/mkfiles/extras.mak))
include $(RPC_ROOT)/src/mkfiles/extras.mak
endif
