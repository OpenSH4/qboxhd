#!/bin/perl -Wall
# @brief Create an initramfs image from the source directory specified
# by the option -s (default is /src/initramfs).
# The resulting file will be called uinitramfs and will be copied to
# the destination directory specified by the option -b (default is
# /rootfs/<board>/nor/)
# All boards share the same initramfs.
#
# @author Pedro Aguilar <pedro@duolabs.com>
#
# Copyright (C) 2009-2011 Duolabs Spa
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
use Cwd;
use Tie::File;
use Getopt::Long;
use File::Path qw(make_path remove_tree);
use Digest::MD5 qw(md5 md5_hex);
use IO::Compress::Gzip qw(gzip $GzipError) ;

format USAGE =

@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
"Usage: perl $0 -b <board_type> [-s source] [-d destination] [-v version]"

    @<<<<<<<<<<<<<<<<<<< @|| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    "-b <board_type>", ":", "Board name. Values: qboxhd | qboxhd_mini"
    @<<<<<<<<<<<<<<<<<<< @|| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    "-s <source>", ":", "Optional source path."
    @<<<<<<<<<<<<<<<<<<< @|| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    "-d <destination>", ":", "Optional destination path."
    @<<<<<<<<<<<<<<<<<<< @|| @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    "-v <version>", ":", "Optional 3 numbers version. Values: x.y.z"

@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
"Example: perl $0 -b qboxhd_mini -s /my_initramfs_dir -d /tftpboot -v 0.0.1"

.

# Check command line input
my ($board, $version, $help, $dst_path, $src_path);
GetOptions( 'b|board=s' => \$board,
			's|source=s' => \$src_path,
			'd|destination=s' => \$dst_path,
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

if ($version) {
	if ($version !~ /\d+\.\d+\.\d+/) {
		print "\nFATAL: Invalid version format. It must have three numbers. Eg. 1.0.1";
		write and exit;
	}
}

my $curdir = getcwd();

if (!$src_path) {
	# Default source: /src/initramfs
	$src_path = "../src/initramfs";
}

if (!$dst_path) {
	# Default destination: /rootfs/<board>/nor/
	$dst_path = "../rootfs/".$board."/rootfs/nor";
}

# Build_path = /build/<board_type>/
my $build_path = $curdir."/../build/".$board;

# Check that paths exist
die "FATAL: Source path '$src_path' does not exists. Aborting!\n" unless (-e $src_path);
if (! -e $dst_path) {
	mkpath($dst_path, { error => \my $err });
	if (@$err) {
		for my $diag (@$err) {
			my ($file, $message) = %$diag;
			die "FATAL: mkpath(): $message\n";
		}
	}
}

my ($login, $pass, $uid, $gid) = getpwnam(getlogin());

tie my @lines, 'Tie::File', "$src_path/init" or die "FATAL: Couldn't open $src_path/init\n";
my ($v1, $v2, $v3);
foreach my $line (@lines) {
    if ($line =~ /^\s*INITRAMFS_VER\s*\=\s*(\d+\.\d+\.\d+)\s*$/) {
		($v1, $v2, $v3) = split(/\./, $1);
		if ($version) {
			($v1, $v2, $v3) = split(/\./, $version);
		}
		else {
			$v3++;
			$version = "$v1.$v2.$v3";
		}
		#print "INITRAMFS_VER=$v1.$v2.$v3";
		$line = "INITRAMFS_VER=$v1.$v2.$v3";
		last;
	}
}
untie @lines;

print "Copying and creating cpio file...";
`rsync -az --exclude=*.svn $src_path $build_path/`;
chdir("$build_path/initramfs");
`find . -print0 | cpio -o -0 --format=newc -O $build_path/initramfs.cpio`;
print "Compressing...";
gzip "$build_path/initramfs.cpio" => "$build_path/initramfs.cpio.gz",-Level => 9 
	or die "FATAL: gzip failed: $GzipError. Aborting\n";
chdir($curdir);

die "FATAL: File initramfs.cpio.gz does not exit in '$build_path'. Aborting\n!" unless (-e "$build_path/initramfs.cpio.gz");

`./mkimage -A sh4 -O linux -T ramdisk -a 0x84000000 -e 0x84000000 -n "QBoxHD initramfs" -d $build_path/initramfs.cpio.gz $dst_path/uinitramfs`;

# Calculate md5 digest
print "Calculating MD5 digest...";
open(my $fd, '<', "$dst_path/uinitramfs") or die $!;
read $fd, my $md5data, 1000000000;
my $digest = md5_hex($md5data);
close($fd);

print "Cleaning...";
remove_tree("$build_path/initramfs", { error => \my $err});
if (@$err) {
	for my $diag (@$err) {
		my ($file, $message) = %$diag;
		print "WARNING: remove_tree(): $message\n";
	}
}
my @goners = ("$build_path/initramfs.cpio", "$build_path/initramfs.cpio.gz");
unlink @goners or warn "WARNING: Could not clean files: $!";

#`rm -rf $build_path/initramfs`;

format REPORT =

=========================================================================================
   @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<
   "initramfs image created for the", uc($board)
   @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<
   "Version", $version
   @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   "Source path", $src_path
   @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   "Destination path", "$dst_path/uinitramfs"
   @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   "MD5 digest", $digest
=========================================================================================

.

$~ = "REPORT";
write;

