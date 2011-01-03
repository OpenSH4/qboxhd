#!/bin/perl -Wall
# @brief Create a tbz compressed stable/beta QBoxHD update that contains 
# a whole filesystem rootfs.cpio, the NOR flash, the update script update.sh, 
# the ChangeLog.txt and README.txt files.
# All unneeded files such as *.pyo are removed and libraries stripped down.
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
use Digest::MD5 qw(md5 md5_hex);
use LWP::UserAgent;
use HTTP::Request::Common;
use Getopt::Long;
use File::Basename;

format USAGE =

@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
"Usage: perl $0 <options>"

    @<<<<<<<<<<<<<<<<<<<<<< @|| @>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    "-b <board_type>", ":", "Board name. Values: qboxhd | qboxhd_mini"
    @<<<<<<<<<<<<<<<<<<<<<< @|| @>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    "-r <release>", ":", "Type of filesystem release. Values: stable | beta"
    @<<<<<<<<<<<<<<<<<<<<<< @|| @>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    "-v <version>", ":", "Optional 3 numbers version. Values: x.y.z"

@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
"Example: perl $0 -b qboxhd_mini -r beta -v 0.0.1"

.

###
### Check command line input
###

my ($board, $release, $version, $help, $path);
GetOptions( 'b|board=s' => \$board,
			'r|release=s' => \$release,
			'v|version=s' => \$version,
			'h|help' => \$help
		);

$~ = 'USAGE';

write and exit if ($help);

if (!$board) {
	print "\nFATAL: NULL board type";
	write and exit;
}
elsif ($board !~ /^qboxhd$/ and $board !~ /^qboxhd_mini$/) {
	print "\nFATAL: Invalid board type '$board'";
	write and exit;
}

if (!$release) {
	print "\nFATAL: NULL release type";
	write and exit;
}
if ($release !~ /^stable$/ and $release !~ /^beta$/) {
	print "\nFATAL: Invalid release type '$release'";
	write and exit;
}

if ($version !~ /\d+\.\d+\.\d+/) {
	print "\nFATAL: Invalid version format. It must have three numbers. Eg. 1.0.1";
	write and exit;
}

my $curdir = getcwd();

# build_path = /releases/<board_type>/qboxhd_update
my $build_path = $curdir."/../releases/".$board."/qboxhd_update";
print "Build path '$build_path' already exists. Aborting..." if (-e $build_path);
mkpath($build_path, { error => \my $err });
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
$build_path =~ /\/qboxhd_update$/;
my $download_path = $`;
$download_path = "./" if ($download_path !~ /\w/);

# rootfs_path = /rootfs/<board_type>/rootfs
my $rootfs_path = $curdir."/../rootfs/".$board."/rootfs";

format HEADER =

=========================================================================================
   @<<<<<<<<< @|||||||| @>>>>>>>>>>>>>> @<<<<<<<<<<<<<<<<<<<<<<<<<<
   "Creating a", uc($release), "package for the", uc($board)
=========================================================================================

.

$~ = 'HEADER';
write;

my ($login, $pass, $uid, $gid) = getpwnam(getlogin());

print "build_path: $build_path";
print "rootfs_path: $rootfs_path";
print "download_path: $download_path";

`cp -a $rootfs_path/../nor $rootfs_path/../*.txt $rootfs_path/../update.sh $build_path`;


if ( ! -e $build_path."/update.sh" or ! -x $build_path."/update.sh") {
	die "FATAL: update.sh script not found or cannot be executed. Aborting!\n"
}

if ( ! -e $build_path."/README.txt" ) {
	die "FATAL: README file not found. Aborting!\n"
}

if ( ! -e $build_path."/ChangeLog.txt" ) {
	die "FATAL: ChangeLog.txt file not found. Aborting!\n"
}

# Abort if rcS exports the QBOXHD_ENV variable 
my $target_file = $rootfs_path."/etc/init.d/rcS";
tie my @lines, 'Tie::File', $target_file or die "FATAL: Couldn't open $target_file\n";
foreach my $line (@lines) {
	die "FATAL: File '$target_file' is for DEVELOPMENT only!!!\n" if $line =~ /^export\s+QBOXHD_ENV\=/;
}
untie @lines;

# Write the creation time and date of the release to file /etc/image-version
$target_file = $rootfs_path."/etc/image-version";
tie @lines, 'Tie::File', $target_file or die "FATAL: Couldn't open $target_file\n";

$lines[0] =~ /\=\s*\d{4}/;
my $ver = $&;
$ver =~ s/\=//;
$ver++;

my @abbr = qw( Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec );
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time);

$year += 1900;
$hour = "0".$hour if ($hour < 10);
$min = "0".$min if ($min < 10);
$lines[0] = "version=".$ver.$abbr[$mon]." ".$mday." ".$year." ".$hour.":".$min;
print "Writing time and date in /etc/image-version: $lines[0]";
untie @lines;

# Write filesystem release type and version in /etc/fs-version
tie @lines, 'Tie::File', "$rootfs_path/etc/fs-version" or die "FATAL: Couldn't open fs-version\n";
my ($v1, $v2, $v3);
foreach my $line (@lines) {
	if ($line =~ /^\s*(stable|beta)\s*(\d+\.\d+\.\d+)\s*$/) {
		($v1, $v2, $v3) = split(/\./, $2);
		if ($version) {
			($v1, $v2, $v3) = split(/\./, $version);
		}
		else {
			$v3++; 
			$version = "$v1.$v2.$v3";
		}
		$line = "$release $v1.$v2.$v3";
	}
}
print "Writing filesystem version in /etc/fs-version: $version";
untie @lines;

# Remove unwanted stuff
print "Removing SVN metadata";
`find $build_path -name .svn | xargs rm -rf `;

print "Removing Python tests dirs";
`find $rootfs_path/usr/local/lib/python2.6 -name test | xargs rm -rf`;
`find $rootfs_path/usr/local/lib/python2.6 -name tests | xargs rm -rf`;

print "Removing Python *.pyo files";
`find $rootfs_path -name *.pyo | xargs rm -rf`;

print "Removing Python *.pyc files";
my @pyc = `find $rootfs_path -name *.pyc`;
foreach my $pyc (@pyc) {
	next if ($pyc =~ /YouPornInterface\.pyc/);
	next if ($pyc =~ /MaxCryptoInfo\.pyc/);
	next if ($pyc =~ /MaxServiceInfo\.pyc/);
	next if ($pyc =~ /vSize\.pyc/);
	next if ($pyc =~ /iDevsDevice\.pyc/);
	next if ($pyc =~ /iDevsDeviceFileList\.pyc/);
	chomp($pyc);
	unlink($pyc);
}

print "Removing unwanted Python *.py files";
unlink "$rootfs_path/usr/local/lib/enigma2/python/Plugins/Extensions/YouPornPlayer/YouPornInterface.py";

# Strip libs and binaries
print "Stripping libraries";
`find $rootfs_path -name *.so | xargs sh4-linux-strip`;

print "Stripping binaries";
my @bin_dirs = ("/bin", "/sbin", "/usr/bin", "/usr/sbin", "/usr/local/bin");
foreach my $bin (@bin_dirs) {
	$bin = $rootfs_path.$bin;
	opendir(DIR, $bin) or die "FATAL: Couldn't open '$bin'\n";
	my @files = readdir(DIR);

	foreach my $file (@files) {
		next if ( -l $file or -d $file);
		next if ($file =~ /MAKEDEV/);
		next if ($file =~ /gtk-games/);
		next if ($file =~ /gpg-error-config/);
		next if ($file =~ /libgcrypt-config/);
		next if ($file =~ /usb_detect/);
		next if ($file =~ /usb_hdd_detect/);
		next if ($file =~ /usb_idev_detect/);
		`sh4-linux-strip $bin/$file`;
	}
	closedir(DIR);
}

# Create the .tbz
print "Creating release";
print "Please wait...";
chdir($rootfs_path);
`find -path *.svn -prune -o -print0 | cpio -o -0 --format=newc > $build_path/rootfs.cpio`;
chdir($curdir);

print "Compressing release";
my $release_filename;
if ($release =~ /^stable$/) {
	if ($board =~ /^qboxhd$/) {
		$release_filename = "qboxhd_update_";
	}
	else {
		$release_filename = "qboxhd_mini_update_";
	}
}
else {
	# We have a beta release
	if ($board =~ /^qboxhd$/) {
		$release_filename = "qboxhd_update_beta_";
	}
	else {
		$release_filename = "qboxhd_mini_update_beta_";
	}
}

$release_filename .= "$v1.$v2.$v3.tbz";

chdir($download_path);
`tar -c -v -j --exclude=rootfs -f $release_filename qboxhd_update`;
chdir($curdir);

chown $uid, $gid, "$download_path/$release_filename"; 

# Calculate md5 digest
print "Calculating MD5 digest";
open(my $fd, '<', "$download_path/$release_filename") or die $!;
read $fd, my $md5data, 1000000000;
my $digest = md5_hex($md5data);
close($fd);

format REPORT =

=========================================================================================
   @<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   "Release name", $release_filename
   @<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   "MD5 digest", $digest
=========================================================================================

.

$~ = "REPORT";
write;

