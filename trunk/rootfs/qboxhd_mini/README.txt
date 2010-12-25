
================================= QBOX HD mini =================================

This package contains the official BETA upgrade package for the QBoxHD mini.


Please visit the official Duolabs forum at www.duolabs.com/forumz 
for feedback on this QBoxHD mini software release.
Your help will be greatly appreciated!


Disclaimer
=============================

This is a new firmware distribution for QBoxHD mini.
This release is still BETA and there are some known bugs.
The purpose for this release is to detect and report bugs and 
eventually become a stable release.

Some functionalities may or may NOT work or cause crashes. Please report 
behaviors on the aforementioned Forum or contact Duolabs technical support.
We remind you that this version is BETA. 


Instructions
=============================

1.  Copy the compressed file qboxhd_update_mini_ddmmyy_xxx.tbz 
	to a USB flash drive.
2.  Optional but strongly recommended: If you want a faster update procedure, 
	uncompress the file inside the USB flash drive. 
	This will create a qboxhd_update_mini directory.
3.  Turn off the QBoxHD mini.
4.  Insert the USB flash drive that contains the update file.
5.  Turn on the QBoxHD mini.
6.  The update procedure starts. The LCD will display the update status.
	If you decompressed the file before, this operation takes approximately
	<5 min. If you didn't decompress it, it can take up to 15 min.
7.  When the update procedure finishes, the QBoxHD mini will continue with
	the normal start-up.
8.  Remove the USB flash drive that contains the update package.


Update contents
=============================
.
|-- ChangeLog.txt			Changes since last version
|-- README.txt				This file
|-- rootfs.cpio				Update filesystem image
|-- update.sh				Update script
`-- nor						Directory with files for the NOR flash
	|-- qboxhdinfo_proc.ko  Driver for gathering decoder's info 
	|-- reset_st40  		Reset utility
	|-- uboot  				Boot-loader
	|-- uimage				Kernel image
	`-- uinitramfs			initramfs image

