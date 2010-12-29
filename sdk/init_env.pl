#!/bin/perl -Wall
#
# @brief Configure and create the paths used for building the sources
# and mount an NFS filesystem.
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
use Net::Ping;
use Cwd;

tie my @lines, 'Tie::File', "../config.in" or die "FATAL: Couldn't open 'config.in'\n";

my $curdir = getcwd();
my $duolabs_host = "devserver";

my %config;
my $err;
foreach my $line (@lines) {
	if ($line =~ /^\s*ROOT_DIR\s*\=\s*/ ) {
		$config{'rootdir'} = $';
	}
	elsif ($line =~ /^\s*REPO_URL\s*\=\s*/ ) {
		$config{'url'} = $';
	}
	elsif ($line =~ /^\s*REPO_URL_DUO\s*\=\s*/ ) {
		$config{'url_duo'} = $';
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
	elsif ($line =~ /^\s*REPO_INCLUDE\s*\=\s*/) {
		$config{'url_include'} = $';
	}
	elsif ($line =~ /^\s*REPO_PTI\s*\=\s*/) {
		$config{'url_pti'} = $';
	}
	elsif ($line =~ /^\s*REPO_SMARTCARD\s*\=\s*/) {
		$config{'url_smartcard'} = $';
	}
	elsif ($line =~ /^\s*REPO_STARCI2WIN\s*\=\s*/) {
		$config{'url_starci2win'} = $';
	}
	elsif ($line =~ /^\s*REPO_QBOXHDINFO\s*\=\s*/) {
		$config{'url_qboxhdinfo'} = $';
	}
	elsif ($line =~ /^\s*REPO_DELAYER\s*\=\s*/) {
		$config{'url_delayer'} = $';
	}
	elsif ($line =~ /^\s*REPO_PROTOCOL\s*\=\s*/) {
		$config{'url_protocol'} = $';
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
	elsif ($line =~ /SRC_INCLUDE\s*\=\s*/) {
		$config{'src_include'} = $';
	}
	elsif ($line =~ /SRC_PTI\s*\=\s*/) {
		$config{'src_pti'} = $';
	}
	elsif ($line =~ /SRC_SMARTCARD\s*\=\s*/) {
		$config{'src_smartcard'} = $';
	}
	elsif ($line =~ /SRC_STARCI2WIN\s*\=\s*/) {
		$config{'src_starci2win'} = $';
	}
	elsif ($line =~ /SRC_QBOXHDINFO\s*\=\s*/) {
		$config{'src_qboxhdinfo'} = $';
	}
	elsif ($line =~ /SRC_DELAYER\s*\=\s*/) {
		$config{'src_delayer'} = $';
	}
	elsif ($line =~ /SRC_PROTOCOL\s*\=\s*/) {
		$config{'src_protocol'} = $';
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
	$value =~ s/\$\(SRC_DRIVERS\)/$config{'src_drivers'}/;
	$value =~ s/\$\(REPO_URL\)/$config{'url'}/;
	$value =~ s/\$\(REPO_URL_DUO\)/$config{'url_duo'}/;
	$value =~ s/\$\(ROOT_DIR\)/$config{'rootdir'}/;

	$config{$key} = $value;
}

# Debug
#while (my($key, $value) = each(%config)) {
	#print "$key -> $value";
#}

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
### Checkout sources
###
my $p = Net::Ping->new();
if ($p->ping($duolabs_host)) {
	print "\n\nChecking out 'include'\nURL: $config{'url_include'}\nDestination: $config{'src_include'}";
	if (-e "$config{'src_include'}") {
		print "Driver already exists. Skipping...";
	}
	else {
		#print "svn checkout $config{'url_include'} $config{'src_include'}";
		my $svn_ret = `svn checkout $config{'url_include'} $config{'src_include'}`;
		print "WARNING: Merging OSS and proprietary files in $config{'src_include'}/../includes";
		`rsync -az --exclude=*.svn $config{'src_include'}/ $config{'src_include'}/../includes/`
	}

	print "\n\nChecking out 'pti'\nURL: $config{'url_pti'}\nDestination: $config{'src_pti'}";
	if (-e "$config{'src_pti'}") {
		print "Driver already exists. Skipping...";
	}
	else {
		#print "svn checkout $config{'url_pti'} $config{'src_pti'}";
		my $svn_ret = `svn checkout $config{'url_pti'} $config{'src_pti'}`;
	}

	print "\n\nChecking out 'smartcard'\nURL: $config{'url_smartcard'}\nDestination: $config{'src_smartcard'}";
	if (-e "$config{'src_smartcard'}") {
		print "Driver already exists. Skipping...";
	}
	else {
		#print "svn checkout $config{'url_smartcard'} $config{'src_smartcard'}";
		my $svn_ret = `svn checkout $config{'url_smartcard'} $config{'src_smartcard'}`;
	}

	print "\n\nChecking out 'starci2win'\nURL: $config{'url_starci2win'}\nDestination: $config{'src_starci2win'}";
	if (-e "$config{'src_starci2win'}") {
		print "Driver already exists. Skipping...";
	}
	else {
		#print "svn checkout $config{'url_starci2win'} $config{'src_starci2win'}";
		my $svn_ret = `svn checkout $config{'url_starci2win'} $config{'src_starci2win'}`;
	}

	print "\n\nChecking out 'qboxhdinfo'\nURL: $config{'url_qboxhdinfo'}\nDestination: $config{'src_qboxhdinfo'}";
	if (-e "$config{'src_qboxhdinfo'}") {
		print "Driver already exists. Skipping...";
	}
	else {
		#print "svn checkout $config{'url_qboxhdinfo'} $config{'src_qboxhdinfo'}";
		my $svn_ret = `svn checkout $config{'url_qboxhdinfo'} $config{'src_qboxhdinfo'}`;
	}

	print "\n\nChecking out 'delayer'\nURL: $config{'url_delayer'}\nDestination: $config{'src_delayer'}";
	if (-e "$config{'src_delayer'}") {
		print "Driver already exists. Skipping...";
	}
	else {
		#print "svn checkout $config{'url_delayer'} $config{'src_delayer'}";
		my $svn_ret = `svn checkout $config{'url_delayer'} $config{'src_delayer'}`;
	}

	print "\n\nChecking out 'protocol'\nURL: $config{'url_protocol'}\nDestination: $config{'src_protocol'}";
	if (-e "$config{'src_protocol'}") {
		print "Driver already exists. Skipping...";
	}
	else {
		#print "svn checkout $config{'url_protocol'} $config{'src_protocol'}";
		my $svn_ret = `svn checkout $config{'url_protocol'} $config{'src_protocol'}`;
	}
}
$p->close();

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

tie @lines, 'Tie::File', $nfs_dir."/qboxhd/etc/network/interfaces" or 
	die "FATAL: Couldn't open '$nfs_dir/qboxhd/etc/network/interfaces'\n";
foreach my $line (@lines) {
	$line = "iface eth0 inet static" if ($line =~ /iface eth0 inet dhcp/);
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

tie @lines, 'Tie::File', $nfs_dir."/qboxhd_mini/etc/network/interfaces" or 
	die "FATAL: Couldn't open '$nfs_dir/qboxhd_mini/etc/network/interfaces'\n";
foreach my $line (@lines) {
	$line = "iface eth0 inet static" if ($line =~ /iface eth0 inet dhcp/);
}
untie @lines;

print "\nDone!\n"

