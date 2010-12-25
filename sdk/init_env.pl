#!/bin/perl -Wall
#
# @brief Configure and create the paths used for compiling the sources.
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
#

use strict;
use Tie::File;
use File::Path;
use File::Copy;
use Cwd;

tie my @lines, 'Tie::File', "../config.in" or die "FATAL: Couldn't open 'config.in'\n";

my $curdir = getcwd();

my %config;
my $err;
foreach my $line (@lines) {
	if ($line =~ /^\s*ROOT_DIR\s*\=\s*/ ) {
		$config{'rootdir'} = $';
	}
	elsif ($line =~ /^\s*REPO_URL\s*\=\s*/ ) {
		$config{'url'} = $';
	}
	elsif ($line =~ /^\s*VER\s*\=\s*/ ) {
		$config{'version'} = $';
	}
	elsif ($line =~ /^\s*TFTPBOOT\s*\=\s*/ ) {
		$config{'tftpboot'} = $';
	}


	elsif ($line =~ /^\s*REPO_KERNEL\s*\=\s*/) {
		$config{'url_kernel'} = $';
	}
	elsif ($line =~ /^\s*REPO_DRIVERS\s*\=\s*/) {
		$config{'url_drivers'} = $';
	}
	elsif ($line =~ /^\s*REPO_APPS\s*\=\s*/) {
		$config{'url_apps'} = $';
	}
	elsif ($line =~ /^\s*REPO_E2\s*\=\s*/) {
		$config{'url_e2'} = $';
	}
	elsif ($line =~ /^\s*REPO_UBOOT\s*\=\s*/) {
		$config{'url_uboot'} = $';
	}
	elsif ($line =~ /^\s*REPO_GENESIS\s*\=\s*/) {
		$config{'url_genesis'} = $';
	}
	elsif ($line =~ /^\s*REPO_ROOTFS_QBOXHD\s*\=\s*/) {
		$config{'url_rootfs_qboxhd'} = $';
	}
	elsif ($line =~ /^\s*REPO_ROOTFS_QBOXHD_MINI\s*\=\s*/) {
		$config{'url_rootfs_qboxhd_mini'} = $';
	}


	elsif ($line =~ /SRC_KERNEL\s*\=\s*/) {
		$config{'src_kernel'} = $';
	}
	elsif ($line =~ /SRC_E2\s*\=\s*/) {
		$config{'src_e2'} = $';
	}
	elsif ($line =~ /SRC_DRIVERS\s*\=\s*/) {
		$config{'src_drivers'} = $';
	}
	elsif ($line =~ /SRC_APPS\s*\=\s*/) {
		$config{'src_apps'} = $';
	}
	elsif ($line =~ /SRC_UBOOT\s*\=\s*/) {
		$config{'src_uboot'} = $';
	}
	elsif ($line =~ /SRC_GENESIS\s*\=\s*/) {
		$config{'src_genesis'} = $';
	}
	elsif ($line =~ /ROOTFS\s*\=\s*/) {
		$config{'src_rootfs'} = $';
	}
	elsif ($line =~ /ROOTFS_NFS\s*\=\s*/) {
		$config{'rootfs_nfs'} = $';
	}
}
untie @lines;

if ($config{'rootdir'} =~ /\$\(PWD\)/) {
	$config{'rootdir'} = getcwd();
	$config{'rootdir'} =~ s/\/sdk\s*$//;
}
else {
	print "SDK path '$config{'rootdir'}' already exists. Aborting..." if (-e $config{'rootdir'});
	mkpath($config{'rootdir'}, { error => \$err }); 
	if (@$err) {
		for my $diag (@$err) {
			my ($file, $message) = %$diag;
			if ($file eq '') {
				print "General error: $message\n";
			}   
			else {
				print "$file: $message\n";
			}   
		}   
	}
	`cp config.in init_env.pl Makefile $config{'rootdir'}`;
}


if ($config{'url'} and $config{'rootdir'} and $config{'version'}) {

format STDOUT =

=========================================================================================
   @<<<<<<<<<<<<<<<<<<< @| @<<<<<<<<<<
   "Build system", ":", $config{'version'}
   @<<<<<<<<<<<<<<<<<<< @| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   "Installation path", ":", $config{'rootdir'}
   @<<<<<<<<<<<<<<<<<<< @| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   "OSS server", ":", $config{'url'}
=========================================================================================
.

write;

}
else {
	die "FATAL: Missing version, root directory or URL";
}


while (my($key, $value) = each(%config)) {
	$value =~ s/\$\(REPO_URL\)/$config{'url'}/;
	$value =~ s/\$\(ROOT_DIR\)/$config{'rootdir'}/;

	$config{$key} = $value;
}

# Debug
while (my($key, $value) = each(%config)) {
	print "$key -> $value";
}

###
### Set enigma2 source path and prefix in the script that compiles it
###
tie @lines, 'Tie::File', "e2.pl" or die "FATAL: Couldn't open 'e2.pl'\n";
foreach my $line (@lines) {
	$line="my \$SRC_E2 = \"$config{'src_e2'}\";\n" if ($line =~ /^my \$SRC_E2/);
	$line="my \$PREFIX = \"$config{'rootfs_nfs'}\";\n" if ($line =~ /^my \$PREFIX/);
}
untie @lines;

###
### Checkout sources from SVN repo
###

#print "\n\nChecking out 'drivers'\nURL: $config{'url_drivers'}\nDestination: $config{'src_drivers'}";
#if (-e $config{'src_drivers'}) {
	#print "Repository already exists. Skipping...";
#}
#else {
	##print "svn checkout $config{'url_drivers'} $config{'src_drivers'}";
	#my $svn_ret = `svn checkout $config{'url_drivers'} $config{'src_drivers'}`;
#}


#print "\n\nChecking out 'applications'\nURL: $config{'url_apps'}\nDestination: $config{'src_apps'}";
#if (-e $config{'src_apps'}) {
	#print "Repository already exists. Skipping...";
#}
#else {
	##print "svn checkout $config{'url_apps'} $config{'src_apps'}";
	#my $svn_ret = `svn checkout $config{'url_apps'} $config{'src_apps'}`;
#}


###
### Create build dirs
###
my $builddir = $config{'rootdir'}."/build";
`mkdir -p $builddir` if (! -e $builddir);

$builddir = $config{'rootdir'}."/build/qboxhd";
`mkdir -p $builddir` if (! -e $builddir);

$builddir = $config{'rootdir'}."/build/qboxhd_mini";
`mkdir -p $builddir` if (! -e $builddir);

###
### Populate NFS dirs
###
my $nfs_dir = $config{'rootdir'}."/nfs";
my $rootfs_dir = $config{'rootdir'}."/rootfs";

print "\n\nPopulating NFS target filesystem for QBoxHD\nDestination: $nfs_dir/qboxhd";
`rsync -az --exclude-from=rsync_exclude.txt $rootfs_dir/qboxhd/rootfs/ $nfs_dir/qboxhd`;

tie @lines, 'Tie::File', $nfs_dir."/qboxhd/etc/init.d/rcS" or 
	die "FATAL: Couldn't open '$nfs_dir/qboxhd/etc/init.d/rcS'\n";
foreach my $line (@lines) {
	last if ($line =~ /export QBOXHD_ENV/);
	$line .= "\nexport QBOXHD_ENV=devel" if ($line =~ /^\#\!\/bin\/sh/);
}
untie @lines;

print "\n\nPopulating NFS target filesystem for QBoxHD mini\nDestination: $nfs_dir/qboxhd_mini";
`rsync -az --exclude-from=rsync_exclude.txt $rootfs_dir/qboxhd_mini/rootfs/ $nfs_dir/qboxhd_mini`;

tie @lines, 'Tie::File', $nfs_dir."/qboxhd_mini/etc/init.d/rcS" or 
	die "FATAL: Couldn't open '$nfs_dir/qboxhd_mini/etc/init.d/rcS'\n";
foreach my $line (@lines) {
	last if ($line =~ /export QBOXHD_ENV/);
	$line .= "\nexport QBOXHD_ENV=devel" if ($line =~ /^\#\!\/bin\/sh/);
}
untie @lines;


print "\nDone!\n"

