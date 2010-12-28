#!/bin/perl -Wall
#
# @brief Builds enigma2
#
# @author Pedro Aguilar <pedro@duolabs.com>
#
# Copyright (C) 2009-2010 Duolabs Spa
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# WARNING: The variables $SRC_E2 and $PREFIX are automatically 
# modified by init_env.pl
#

use strict;
use Tie::File;

# WARNING!!!
# Vars edited by init_env.pl
my $SRC_E2 = "/home/projects/qboxhd/qboxhd_sdk_oss/src/apps/enigma2";
my $PREFIX = "/home/projects/qboxhd/qboxhd_sdk_oss/nfs";

sub usage()
{
format STDOUT =
Usage: $0 <op> <board>";
@>>>>>>>>>> @>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
"op", "build | install | clean | distclean"
@>>>>>>>>>> @>>>>>>>>>>>>>>>>>>>>>>>>>>
"board", "qboxhd | qboxhd_mini"
.

write;
}

sub e2_build()
{
	my $board = shift(@_);

	my $BUILD = "i386-linux";
	my $HOST = "sh4-linux";

	$PREFIX .= "/$board";

	my $e2_config = " \\
	PYTHON=/usr/bin/python \\
	./configure \\
	--build=$BUILD \\
	--host=$HOST \\
	--with-libsdl=no \\
	--with-dvbincludes=$PREFIX/usr/include \\
	--with-datadir=/usr/local/share \\
	--with-tuxboxdatadir=/usr/local/share/tuxbox \\
	--with-libdir=/usr/local/lib \\
	PKG_CONFIG=\"/opt/STM/STLinux-2.3/host/bin/pkg-config --define-variable=prefix=$PREFIX/usr/local\" \\
	PKG_CONFIG_PATH=$PREFIX/usr/local/lib/pkgconfig";


	if (! -d $SRC_E2) {
		die "FATAL: Enigma2 source directory '$SRC_E2' doesn't exist. Aborting...";
	}

	my $CC=`sh4-linux-gcc --version`;
	if (!$CC) {
		die "FATAL: Compiler not found. Please add it to your \$PATH. Aborting..."
	}

	chdir($SRC_E2);
	# First build?
	if ( ! -e "main/Makefile") {
		if ( ! -e "configure") {
			my $pkg_config = 0;

			print "==================================================";
			print "Executing ./autogen.sh";
			`./autogen.sh 1>&2`;
			# TODO: Fix these hacks 
			tie my @lines, 'Tie::File', "configure" or die "FATAL: Couldn't open 'configure'\n";
			foreach my $line (@lines) {
				if ($line =~ /^\s*PYTHON_VERSION\=\$am_cv_python_version\s*$/) {
					$line = "PYTHON_VERSION=2.6";
				}
				if ($line =~ /^\s*QBOXHD_PYTHON_CPPFLAGS\=/) {
					$line = "QBOXHD_PYTHON_CPPFLAGS=\"-I$PREFIX/usr/local/include\"";
				}
				if ($line =~ /^\s*QBOXHD_PYTHON_LDFLAGS\=/) {
					$line = "QBOXHD_PYTHON_LDFLAGS=\"-L$PREFIX/usr/local/lib -L$PREFIX/usr/local/lib/python2.6 -L$PREFIX/usr/local/lib/python2.6/site-packages\"";
				}
				if (($line =~ /^\s*\_pkg_min_version\=/) || ($pkg_config > 0 && $pkg_config < 11)) {
						$line = "#".$line if ($pkg_config > 0);
						$pkg_config++;
				}
			}
			untie @lines;
		}

		if ($board =~ /qboxhd_mini/) {
			$e2_config .= " --with-qboxhd-mini";
		}
		print "==================================================";
		print "Executing ./configure with following parameters: \n$e2_config";
		#`./configure $e2_config 1>&2`
		`$e2_config 1>&2`
	}
	else {
		my $prev_board = "qboxhd";

		tie my @lines, 'Tie::File', "qboxhd.py" or die "FATAL: Couldn't open 'qboxhd.py";
		foreach my $line (@lines) {
			if ($line =~ /QBOXHD_MINI \= 1/) {
				$prev_board = "qboxhd_mini";
				last;
			}
		}
		untie @lines;

		# We're building for a different board
		if ($board !~ /^$prev_board$/) {
			&e2_distclean($board);
			&e2_build($board);
			return;
		}
	}

	print "==================================================";
	print "Compiling...";
	`make 1>&2`;
	print "Done!";
}

sub e2_install()
{
	my $board = shift(@_);

	$PREFIX .= "/$board";

	chdir($SRC_E2);
	print "==================================================";
	print "Installing...";
	`make DESTDIR=$PREFIX install 1>&2`;
	print "Done!";
}

sub e2_clean()
{
	my $board = shift(@_);

	chdir($SRC_E2);
	print "==================================================";
	print "Cleaning...";
	`make clean 1>&2`;
	print "Done!";
}

sub e2_distclean()
{
	my $board = shift(@_);

	chdir($SRC_E2);
	print "==================================================";
	print "Distcleaning...";
	`make distclean 1>&2`;
	unlink "configure";
	print "Done!";
}


if (!$ARGV[0] || !$ARGV[1]) {
	usage();
	exit 1;
}

my $op = $ARGV[0];
my $board = $ARGV[1];

if ($op =~ /^build$/) 			{ &e2_build($board); }
elsif ($op =~ /^install$/) 		{ &e2_install($board); }
elsif ($op =~ /^clean$/) 		{ &e2_clean($board); }
elsif ($op =~ /^distclean$/) 	{ &e2_distclean($board); }
else 							{ usage(); exit 1; }


