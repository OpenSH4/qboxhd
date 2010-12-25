#
# (C) Copyright 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

CROSS_COMPILE=sh4-linux-

TARGETDIR=/opt/STM/STLinux-2.0/devkit/sh4/target

TOPDIR=../..

CPPFLAGS=-I $(TARGETDIR)/usr/include/ -I$(TOPDIR)/include

SOURCES := crc32.c  fw_env.c  fw_env_main.c
HEADERS := fw_env.h
TARGETS = fw_printenv fw_mkconfig fw_mkconfig_host fw_env.config

INSTALL = /usr/bin/install

all:	$(TARGETS)

fw_printenv:	$(SOURCES) $(HEADERS)
	$(CROSS_COMPILE)gcc $(CPPFLAGS) -Wall -DUSE_HOSTCC $(SOURCES) -o fw_printenv

fw_mkconfig: fw_mkconfig.c
	$(CROSS_COMPILE)gcc -I$(TOPDIR)/include fw_mkconfig.c  -o fw_mkconfig

fw_mkconfig_host: fw_mkconfig.c
	$(CC) -I$(TOPDIR)/include fw_mkconfig.c -o fw_mkconfig_host
	
fw_env.config:	fw_mkconfig_host
	./fw_mkconfig_host > fw_env.config

install: $(TARGETS)
	install -m755 fw_printenv fw_mkconfig $(TARGETDIR)/usr/sbin
	install -m644 fw_env.config $(TARGETDIR)/etc
	if test -e  $(TARGETDIR)/usr/sbin/fw_setenv ; then rm $(TARGETDIR)/usr/sbin/fw_setenv ; fi
	(cd $(TARGETDIR)/usr/sbin/; ln -s fw_printenv fw_setenv)

clean:
	rm -f $(TARGETS) crc32.c 

crc32.c:
	ln -s ../../lib_generic/crc32.c crc32.c

#########################################################################

.depend:	Makefile $(SOURCES) fw_mkconfig.c
		$(CC) -M $(HOST_CFLAGS) $(CPPFLAGS) -DUSE_HOSTCC $(SOURCES) > $@

sinclude .depend

#########################################################################
